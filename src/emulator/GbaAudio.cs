using System;
using static OptimeGBA.Bits;

namespace OptimeGBA
{
    public sealed class CircularBuffer<T>
    {
        public uint Size;
        public T[] Buffer;
        public T EmptyValue;
        public uint ReadPos = 0;
        public uint WritePos = 0;
        public uint Entries = 0;
        public uint TotalPops = 0;
        public uint EmptyPops = 0;
        public uint FullInserts = 0;
        public uint Collisions = 0;
        public byte CurrentByte = 0;

        public CircularBuffer(uint size, T emptyValue)
        {
            Size = size;
            Buffer = new T[Size];
            EmptyValue = emptyValue;
        }

        public void Insert(T data)
        {
            if (Entries < Size)
            {
                if (ReadPos == WritePos) Collisions++;
                Entries++;
                Buffer[WritePos++] = data;
                WritePos %= Size;
            }
            else
            {
                FullInserts++;
            }
        }

        public T Pop()
        {
            T data;
            TotalPops++;
            if (Entries > 0)
            {
                Entries--;
                data = Buffer[ReadPos++];
                ReadPos %= Size;
            }
            else
            {
                EmptyPops++;
                data = EmptyValue;
            }
            return data;
        }

        public void Reset()
        {
            Entries = 0;
            ReadPos = 0;
            WritePos = 0;
        }
    }

    public sealed class GbaAudio
    {
        Gba Gba;
        Scheduler Scheduler;
        public GbaAudio(Gba gba, Scheduler scheduler)
        {
            Gba = gba;
            Scheduler = scheduler;

            Scheduler.AddEventRelative(SchedulerId.ApuSample, SampleTimerMax, Sample);
        }

        public GbAudio GbAudio = new GbAudio();

        public bool DebugEnableA = true;
        public bool DebugEnableB = true;

        public CircularBuffer<byte> A = new CircularBuffer<byte>(32, 0);
        public CircularBuffer<byte> B = new CircularBuffer<byte>(32, 0);

        uint BiasLevel = 0x100;
        uint AmplitudeRes;

        // SOUNDCNT_H
        uint SoundVolume = 0; // 0-1
        bool DmaSoundAVolume = false; // 2
        bool DmaSoundBVolume = false; // 3

        bool DmaSoundAEnableRight = false; // 8
        bool DmaSoundAEnableLeft = false; // 9
        bool DmaSoundATimerSelect = false; // 10

        bool DmaSoundBEnableRight = false; // 11
        bool DmaSoundBEnableLeft = false; // 12
        bool DmaSoundBTimerSelect = false; // 13

        bool MasterEnable = false;

        public byte ReadHwio8(uint addr)
        {


            byte val = 0;
            switch (addr)
            {
                case 0x4000082: // SOUNDCNT_H B0
                    val |= (byte)((SoundVolume >> 0) & 0b11); // 0-1
                    if (DmaSoundAVolume) val = BitSet(val, 2); // 2
                    if (DmaSoundBVolume) val = BitSet(val, 3); // 3
                    break;
                case 0x4000083: // SOUNDCNT_H B1
                    if (DmaSoundAEnableRight) val = BitSet(val, 8 - 8); // 8
                    if (DmaSoundAEnableLeft) val = BitSet(val, 9 - 8); // 9
                    if (DmaSoundATimerSelect) val = BitSet(val, 10 - 8); // 10
                    if (DmaSoundBEnableRight) val = BitSet(val, 12 - 8); // 12
                    if (DmaSoundBEnableLeft) val = BitSet(val, 13 - 8); // 13
                    if (DmaSoundBTimerSelect) val = BitSet(val, 14 - 8); // 14
                    break;

                // Special case, because SOUNDCNT_X contains both GB Audio status and GBA audio status
                case 0x4000084:
                    // NR52
                    byte i = 0;
                    i |= 0b01110000;
                    if (GbAudio.noise_enabled && GbAudio.noise_dacEnabled) i |= (byte)BIT_3;
                    if (GbAudio.wave_enabled && GbAudio.wave_dacEnabled) i |= (byte)BIT_2;
                    if (GbAudio.pulse2_enabled && GbAudio.pulse2_dacEnabled) i |= (byte)BIT_1;
                    if (GbAudio.pulse1_enabled && GbAudio.pulse1_dacEnabled) i |= (byte)BIT_0;

                    if (MasterEnable) i |= (byte)BIT_7;
                    return i;

                case 0x4000088: // SOUNDBIAS B0
                    val |= (byte)(BiasLevel << 1);
                    break;
                case 0x4000089: // SOUNDBIAS B1
                    val |= (byte)(BiasLevel >> 7);
                    val |= (byte)(AmplitudeRes << 6);
                    break;
            }

            if (addr >= 0x4000060 && addr <= 0x4000084)
            {
                // GB Registers
                val = GbAudio.ReadHwio8(addr & 0xFF);
            }
            else if (addr >= 0x4000090 && addr <= 0x400009F)
            {
                // Wave RAM
                val = GbAudio.ReadHwio8(addr & 0xFF);
            }


            return val;
        }

        public void WriteHwio8(uint addr, byte val)
        {
            if (addr >= 0x4000060 && addr <= 0x400009F)
            {
                GbAudio.WriteHwio8(addr & 0xFF, val);
            }

            switch (addr)
            {
                case 0x4000082: // SOUNDCNT_H B0
                    SoundVolume = (uint)(val & 0b11); // 0-1
                    DmaSoundAVolume = BitTest(val, 2); // 2
                    DmaSoundBVolume = BitTest(val, 3); // 3
                    break;
                case 0x4000083: // SOUNDCNT_H B1
                    DmaSoundAEnableRight = BitTest(val, 8 - 8); // 8
                    DmaSoundAEnableLeft = BitTest(val, 9 - 8); // 9
                    DmaSoundATimerSelect = BitTest(val, 10 - 8); // 10
                    if (BitTest(val, 11 - 8)) A.Reset();
                    DmaSoundBEnableRight = BitTest(val, 12 - 8); // 12
                    DmaSoundBEnableLeft = BitTest(val, 13 - 8); // 13
                    DmaSoundBTimerSelect = BitTest(val, 14 - 8); // 14
                    if (BitTest(val, 15 - 8)) B.Reset();
                    break;
                case 0x4000084: // SOUNDCNT_X
                    MasterEnable = BitTest(val, 7);
                    break;
                case 0x4000088: // SOUNDBIAS B0
                    BiasLevel &= 0b110000000;
                    BiasLevel |= (uint)((val >> 1) & 0b1111111);
                    break;
                case 0x4000089: // SOUNDBIAS B1
                    BiasLevel &= 0b001111111;
                    BiasLevel |= (uint)((val & 0b11) << 7);

                    AmplitudeRes &= 0;
                    AmplitudeRes |= (uint)((val >> 6) & 0b11);
                    break;

                case 0x40000A0:
                case 0x40000A1:
                case 0x40000A2:
                case 0x40000A3:
                    // Gba.Arm7.Error("FIFO Insert");
                    A.Insert(val);
                    break;

                case 0x40000A4:
                case 0x40000A5:
                case 0x40000A6:
                case 0x40000A7:
                    // Gba.Arm7.Error("FIFO Insert");
                    B.Insert(val);
                    break;
            }
        }

        public bool CollectSamples = true;

        public bool EnablePsg = true;
        public bool EnableFifo = true;

        const uint SampleTimerMax = 512;
        // public CircularBuffer<short> SampleBuffer = new CircularBuffer<short>(32768, 0);
        public const uint SampleBufferMax = 256;
        public short[] SampleBuffer = new short[SampleBufferMax];
        public uint SampleBufferPos = 0;
        public bool AudioReady;

        public void Sample(long cyclesLate)
        {
            GbAudio.Tick(128); // Tick 128 T-cycles

            short left = 0;
            short right = 0;

            if (MasterEnable)
            {
                if (EnablePsg)
                {
                    left += GbAudio.Out1;
                    right += GbAudio.Out2;
                }
                if (EnableFifo)
                {
                    short a = (short)(DmaSoundAVolume ? (sbyte)A.CurrentByte * 2 : (sbyte)A.CurrentByte * 1);
                    short b = (short)(DmaSoundBVolume ? (sbyte)B.CurrentByte * 2 : (sbyte)B.CurrentByte * 1);
                    if (DebugEnableA)
                    {
                        if (DmaSoundAEnableLeft) left += a;
                        if (DmaSoundAEnableRight) right += a;
                    }
                    if (DebugEnableB)
                    {
                        if (DmaSoundBEnableLeft) left += b;
                        if (DmaSoundBEnableRight) right += b;
                    }
                }
            }

            SampleBuffer[SampleBufferPos++] = ((short)(left * 64));
            SampleBuffer[SampleBufferPos++] = ((short)(right * 64));

            if (SampleBufferPos >= SampleBufferMax)
            {
                SampleBufferPos = 0;

                Gba.AudioCallback(SampleBuffer);
            }

            Scheduler.AddEventRelative(SchedulerId.ApuSample, SampleTimerMax - cyclesLate, Sample);
        }

        public void TimerOverflowFifoA()
        {
            A.CurrentByte = A.Pop();
            if (A.Entries <= 16)
            {
                Gba.Dma.RepeatFifoA();
            }
        }
        public void TimerOverflowFifoB()
        {
            B.CurrentByte = B.Pop();
            if (B.Entries <= 16)
            {
                Gba.Dma.RepeatFifoB();
            }
        }

        // Called when Timer 0 or 1 overflows.
        public void TimerOverflow(uint timerId)
        {
            if (timerId == 0)
            {
                if (!DmaSoundATimerSelect)
                {
                    TimerOverflowFifoA();
                }
                if (!DmaSoundBTimerSelect)
                {
                    TimerOverflowFifoB();
                }
            }
            else if (timerId == 1)
            {
                if (DmaSoundATimerSelect)
                {
                    TimerOverflowFifoA();
                }
                if (DmaSoundBTimerSelect)
                {
                    TimerOverflowFifoB();
                }
            }
        }
    }
}