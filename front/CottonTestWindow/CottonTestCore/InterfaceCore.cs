using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing.Drawing2D;
using System.Drawing;
using System.IO;

namespace CottonTestCoreDynamic
{
    public class InterfaceCore
    {
        public const double UNDEF = 9999999;
        public static class TEMPERATUE
        {
            public const double VREF_R = 1.2;
            public const double VREF_AD = 3.0;
            public const long AD_MAX = (1 << 10) - 1;

            /// <summary>
            /// x : Temperature, ℃.
            /// y : Thermistor Resistance, Ω.
            /// log10(y) = a*x^2+bx+c.
            /// </summary>
            public static class POLY3
            {

                public const double A = 9.0047e-5;
                public const double B = -0.01814533;
                public const double C = 4.3745978;
                public static double calx(double y)
                {
                    return (-B - Math.Sqrt(B * B - 4 * A * (C - Math.Log10(y)))) / (2 * A);
                }
                public static double caly(double x)
                {
                    return Math.Pow(10, A * x * x + B * x + C);
                }
            }
            /// <summary>
            /// Not Recommended.
            /// x : Temperature, ℃.
            /// y : Thermistor Resistance, Ω.
            /// log10(y) = ax+b.
            /// </summary>
            public static class LINEAR
            {
                public const double A = -0.018744142;
                public const double B = 4.4376757;
                public static double calx(double y)
                {
                    return (Math.Log10(y) - B) / A;
                }
                public static double caly(double x)
                {
                    return Math.Pow(10, A * x + B);
                }
            }
  
            /// <summary>
            /// Better performance on a "Consolas" font.
            /// Unit: kΩ
            /// VREF_R
            /// |
            /// |-----
            /// |    |
            /// RX   R2         VREF_AD
            /// |----+--->--(-)    |
            /// |    |     RW-->--ADC----(0~AD_MAX)
            /// |    |--->--(+)    |
            /// R1   R3           GND
            /// |-----
            /// GND
            /// </summary>
            /// <returns></returns>
            public static double cal(long ad_output, double RW = 100, string METHOD = "poly3",
                double R1 = 10, double R2 = 10, double R3 = 10)
            {
                if (RW <= 0 || ad_output == AD_MAX)
                    return UNDEF;
                double ad_input = ((double)ad_output / AD_MAX) * VREF_AD;
                double v_r2r3 = VREF_R * R3 / (R2 + R3);
                double v_rxr1 = v_r2r3 - ad_input / ((double)10 / 3 + 1 / RW * 200 / 3);
                double rx = R1 * (VREF_R / v_rxr1 - 1);
                double ret;
                if (METHOD == "linear")
                    ret = LINEAR.calx(rx * 1000);
                else
                    ret = POLY3.calx(rx * 1000);
                return ret;
            }

            public static double cal_voltage(long ad_output, double RW = 100,
                double R1 = 10, double R2 = 10, double R3 = 10)
            {
                if (RW <= 0 || ad_output == AD_MAX)
                    return UNDEF;
                double ad_input = ((double)ad_output / AD_MAX) * VREF_AD;
                double v_r2r3 = VREF_R * R3 / (R2 + R3);
                double v_rxr1 = v_r2r3 - ad_input / ((double)10 / 3 + 1 / RW * 200 / 3);
                double ret = v_rxr1;
                return ret;
            }

            //TODO:利用电压关系直线计算

            public static List<double> cal_line(List<PointF> pts)
            {
                List<double> ret = new List<double>();
                double[] xs = new double[pts.Count];
                double[] ys = new double[pts.Count];
                for (int i = 0; i < pts.Count; i++)
                {
                    xs[i] = pts[i].X;
                    ys[i] = pts[i].Y;
                }
                var auto_line_para = LeastSquare.MultiLine(xs, ys, pts.Count, 1);
                ret.Add(auto_line_para[0]);
                ret.Add(auto_line_para[1]);
                return ret;
            }

            /// <summary>
            /// 根据拟合直线确定电压值，规定x轴为电压，y轴为AD输出
            /// AD_output = para[0] + para[1] * voltage
            /// </summary>
            /// <param name="ad_output"></param>
            /// <param name="para"></param>
            /// <param name="METHOD"></param>
            /// <param name="R1"></param>
            /// <param name="R2"></param>
            /// <param name="R3"></param>
            /// <returns></returns>
            public static double cal_vol_by_pts(long ad_output, List<double> para)
            {
                double vol = ((double)ad_output - para[0]) / para[1];
                return vol;
            }

            /// <summary>
            /// 根据拟合直线确定温度值，规定x轴为电压，y轴为AD输出
            /// AD_output = para[0] + para[1] * voltage
            /// </summary>
            /// <param name="ad_output"></param>
            /// <param name="para"></param>
            /// <param name="METHOD"></param>
            /// <param name="R1"></param>
            /// <param name="R2"></param>
            /// <param name="R3"></param>
            /// <returns></returns>
            public static double cal_by_pts(long ad_output, List<double> para, string METHOD = "poly3", double R1 = 10)
            {
                double vol = cal_vol_by_pts(ad_output, para);
                double v_rxr1 = vol;
                double rx = R1 * (VREF_R / v_rxr1 - 1);
                double ret;
                if (METHOD == "linear")
                    ret = LINEAR.calx(rx * 1000);
                else
                    ret = POLY3.calx(rx * 1000);
                return ret;
            }

            public static long cal_inv_by_pts(double temperature, List<double> para, string METHOD = "poly3", double R1 = 10)
            {
                double rx;
                if (METHOD == "linear")
                    rx = LINEAR.caly(temperature) / 1000;
                else
                    rx = POLY3.caly(temperature) / 1000;
                double v_rxr1 = VREF_R * R1 / (rx + R1);
                long ret = (long)(para[0] + para[1] * v_rxr1);
                return ret;
            }

            public static long cal_inv(double temperature, double RW = 100, string METHOD = "poly3",
                double R1 = 10, double R2 = 10, double R3 = 10)
            {
                double rx;
                if (METHOD == "linear")
                    rx = LINEAR.caly(temperature) / 1000;
                else
                    rx = POLY3.caly(temperature) / 1000;
                double v_rxr1 = VREF_R * R1 / (rx + R1);
                double v_r2r3 = VREF_R * R3 / (R2 + R3);
                double ad_input = ((double)10 / 3 + 1 / RW * 200 / 3) * (v_r2r3 - v_rxr1);
                if (ad_input >= AD_MAX)
                    ad_input = AD_MAX;
                else if (ad_input <= 0)
                    ad_input = 0;
                long ret = (long)Math.Round((ad_input / VREF_AD) * AD_MAX);
                return ret;
            }

        }
        public static class PHOTODIODE
        {
            public const double VREF_AD = 3.0;
            public const double VREF_R = 5;

            public const long AD_MAX = (1 << 12) - 1;
            public const double OFFSET1 = 0;
            public const double AMP1 = 6;
            public const double OFFSET2 = 0;
            public const double AMP2 = 6;
            public const double OFFSET3 = 0;
            public const double AMP3_R = 150;
            public const double AMP3_RL_MAX = 10;
            public const int AMP3_RL_STEP_MAX = 16;
            public const double SPLIT_RATIO = 0.5;

            public static double cal(long ad_output, double amp3_rl = 10)
            {
                if (amp3_rl <= 0 || ad_output >= AD_MAX || amp3_rl >= AMP3_RL_MAX)
                    return UNDEF;
                double ad_input = (double)ad_output / AD_MAX * VREF_AD;
                double v3_in = ad_input / SPLIT_RATIO / ((AMP3_R + amp3_rl) / amp3_rl);
                double v2_in = (v3_in - OFFSET3) / AMP2;
                double v1_in = (v2_in - OFFSET2) / AMP1;
                double ret = v1_in - OFFSET1;
                return ret;
            }
        }

        /// <summary>
        /// include its number and its points
        /// </summary>
        public class Sensor
        {
            public int sensor_num = -1;
            public List<PointF> temperature_points = new List<PointF>();
            public List<double> temperature_paras = new List<double>();
        }

        /// <summary>
        /// 获取当前连接状态
        /// </summary>
        public bool connected
        {
            get
            {
                return c.connected;
            }
        }

        /// <summary>
        /// 是否打印从服务器收到的数据
        /// </summary>
        public bool print_received = true;

        /// <summary>
        /// 在当前目录下生成样例温度传感器标定配置文件
        /// </summary>
        public void generate_sample_config()
        {
            string str_dir = System.Environment.CurrentDirectory + "/config";
            if (Directory.Exists(str_dir) == false)//如果不存在就创建file文件夹
            {
                Directory.CreateDirectory(str_dir);
            }
            FileStream fs = new FileStream(str_dir + "/config_sample.txt", FileMode.Create);
            StreamWriter sw = new StreamWriter(fs);
            //开始写入
            sw.WriteLine("#这是一个样例温度传感器标定配置文件");
            sw.WriteLine("#使用\"#\"符号作为备注和注释信息，该行信息不会被解读");
            sw.WriteLine("#使用@符号标明传感器的编号，@符号后的数字将作为传感器编号起点");
            sw.WriteLine("#传感器的编号后紧跟2个或更多的X、Y点标定信息，每行的X、Y用空格隔开，X为R1RX点电压，Y为AD输出");
            sw.WriteLine("#下面是一个样例");
            sw.WriteLine("@1");
            sw.WriteLine("0.25 365");
            sw.WriteLine("0.33 456");
            sw.WriteLine("0.54 765");
            sw.WriteLine("@2");
            sw.WriteLine("0.82 214");
            sw.WriteLine("0.54 421");
            sw.WriteLine("@3");
            sw.WriteLine("0.11 46");
            sw.WriteLine("#虽然AD输出一般为整数，但这两个数均是作为float处理的，因此也可以是非整数");
            sw.WriteLine("0.13 49.2");
            //清空缓冲区
            sw.Flush();
            //关闭流
            sw.Close();
            fs.Close();
            Console.WriteLine("样例配置已经生成，请查看程序文件夹下的config文件夹");
            return;
        }

        //读取配置文件
        public void read_config(string file)
        {
            FileStream fs = new FileStream(file, FileMode.Open);
            StreamReader sr = new StreamReader(fs);
            string line;
            Sensor s = new Sensor();
            while ((line = sr.ReadLine()) != null)
            {
                if (line.Replace(" ", "") == "")//除去空行
                    continue;
                if (line[0] == '#')
                    continue;
                else if (line[0] == '@')
                {
                    s = new Sensor();
                    try
                    {
                        s.sensor_num = int.Parse(line.Substring(1));
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                    sensors.Add(s);
                }
                else
                {
                    try
                    {
                        string[] xy = line.Split(new char[] { ' ' });
                        s.temperature_points.Add(new PointF(float.Parse(xy[0]), float.Parse(xy[1])));
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                }
            }
            sr.Close();
            fs.Close();
            for (int i = 0; i < sensors.Count; i++)
            {
                if (sensors[i].temperature_points.Count < 2)
                    throw new Exception("传感器（编号："+sensors[i].sensor_num.ToString()+"）没有足够的数据点");
                sensors[i].temperature_paras = TEMPERATUE.cal_line(sensors[i].temperature_points);
            }
            return;
        }

        /// <summary>
        /// 连接服务器
        /// </summary>
        /// <param name="ip_address"></param>
        /// <param name="port"></param>
        /// <returns></returns>
        public bool connect(string ip_address, int port = 986)
        {
            return c.connect(System.Net.IPAddress.Parse(ip_address), port);
        }

        /// <summary>
        /// 不推荐外界调用此函数
        /// </summary>
        /// <param name="address"></param>
        /// <param name="read"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public Client.ReceiveEventArgs WriteReadReg(uint address, bool read = true, uint value = 0)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            to_send.Add(new byte[] { (read)?(byte)'G':(byte)'S', (byte)'E', (byte)'T', (byte)'X' });
            to_send.Add(BitConverter.GetBytes(address));
            to_send.Add(BitConverter.GetBytes((read) ? 0 : value));
            to_send.Add(BitConverter.GetBytes(0));
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            if (BitConverter.ToUInt32(re.data, 4) == 0)
                throw new Exception("读写寄存器失败");
            print_rev(re);
            return re;
        }

        /// <summary>
        /// 获取板实际温度
        /// </summary>
        /// <param name="num">0,1</param>
        /// <returns></returns>
        public double GetTemperature(int num, int sensor_num = -1)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            uint address = 0xff71 + (uint)num;
            var re = WriteReadReg(address);
            if (sensor_num == -1)
                return TEMPERATUE.cal(BitConverter.ToUInt32(re.data, 8));
            else
            {
                var sensor = sensors.Find(delegate (Sensor s)
                {
                    return s.sensor_num == sensor_num;
                });
                if (sensor == null)
                    throw new Exception("未在配置文件中找到编号为"+ sensor_num.ToString() + "的Sensor");
                return TEMPERATUE.cal_by_pts(BitConverter.ToUInt32(re.data, 8), sensor.temperature_paras);
            }
        }

        /// <summary>
        /// 获取/设置板设定温度
        /// </summary>
        /// <param name="num">0,1</param>
        /// <param name="set"></param>
        /// <param name="temperature">Unit:℃</param>
        /// <returns></returns>
        public double GetSetTemperature(int num, bool set = false, double temperature = -20)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            uint address = 0xff73 + (uint)num;
            Client.ReceiveEventArgs re;
            if (set)
                re = WriteReadReg(address, false, (uint)TEMPERATUE.cal_inv(temperature));
            else
                re = WriteReadReg(address);
            return TEMPERATUE.cal(BitConverter.ToUInt32(re.data, 8));
        }

        /// <summary>
        /// 获取/设置制冷开关状态
        /// </summary>
        /// <param name="num">0,1</param>
        /// <param name="set"></param>
        /// <param name="open"></param>
        /// <returns></returns>
        public bool GetSetCooler(int num, bool set = false, bool open = true)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            uint address = 0xff75 + (uint)num;
            Client.ReceiveEventArgs re;
            if (set)
                re = WriteReadReg(address, false, (uint)(open ? 0 : 1));
            else
                re = WriteReadReg(address);
            return (BitConverter.ToUInt32(re.data, 8) == 0);
        }

        /// <summary>
        /// 获取/设置可调电阻阻值位阶
        /// </summary>
        /// <param name="num">0,1</param>
        /// <param name="set"></param>
        /// <param name="value">当前阻值位阶</param>
        /// <param name="MAX_VALUE">最大阻值位阶</param>
        /// <returns></returns>
        public uint GetSetRisistor(int num, bool set = false,
            int value = PHOTODIODE.AMP3_RL_STEP_MAX, int MAX_VALUE = PHOTODIODE.AMP3_RL_STEP_MAX)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            if (value > MAX_VALUE)
                throw new Exception("设置的电阻值超过了能提供的最大值");
            if (value < 0)
                throw new Exception("设置的电阻值小于零");
            uint address = 0xff77 + (uint)num;
            Client.ReceiveEventArgs re;
            if (set)
                re = WriteReadReg(address, false, (uint)value);
            else
                re = WriteReadReg(address);
            return BitConverter.ToUInt32(re.data, 8);
        }

        /// <summary>
        /// 获取/设置数据采集周期（频率）
        /// 单位：Hz
        /// 应当大于等于显示频率！（默认100）
        /// </summary>
        /// <param name="set"></param>
        /// <param name="freq">周期</param>
        /// <returns></returns>
        public uint GetSetPeriod(bool set = false, int freq = 100)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            uint address = 0xff79;
            Client.ReceiveEventArgs re;
            if (set)
                re = WriteReadReg(address, false, (uint)freq);
            else
                re = WriteReadReg(address);
            return BitConverter.ToUInt32(re.data, 8);
        }

        /// <summary>
        /// 获取/设置数据采集宽度
        /// 单位：AD的时钟
        /// 应当大于等于显示宽度！（默认80）
        /// </summary>
        /// <param name="set"></param>
        /// <param name="width">宽度</param>
        /// <returns></returns>
        public uint GetSetWidth(bool set = false, int width = 80)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            uint address = 0xff80;
            Client.ReceiveEventArgs re;
            if (set)
                re = WriteReadReg(address, false, (uint)width);
            else
                re = WriteReadReg(address);
            return BitConverter.ToUInt32(re.data, 8);
        }

        /// <summary>
        /// 获取/设置触发状态
        /// </summary>
        /// <param name="set"></param>
        /// <param name="trigger"></param>
        /// <returns></returns>
        public bool GetSetTrigger(bool set = false, bool trigger = true)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            uint address = 0xff81;
            Client.ReceiveEventArgs re;
            if (set)
                re = WriteReadReg(address, false, (uint)(trigger ? 1 : 0));
            else
                re = WriteReadReg(address);
            return (BitConverter.ToUInt32(re.data, 8) != 0);
        }

        /// <summary>
        /// 获取传感器当前输入
        /// </summary>
        /// <param name="cal">是否进行电压反算</param>
        /// <returns></returns>
        public KeyValuePair<double, double> GetData(bool cal = true, double r1 = PHOTODIODE.AMP3_RL_MAX, double r2 = PHOTODIODE.AMP3_RL_MAX)
        {
            throw new Exception("[GetData]动态实验中此函数已经废弃");
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            to_send.Add(new byte[] { (byte)'R', (byte)'A', (byte)'W', (byte)'X' });
            to_send.Add(BitConverter.GetBytes(0));
            to_send.Add(BitConverter.GetBytes(0));
            to_send.Add(BitConverter.GetBytes(0));
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            print_rev(re);
            //请注意：此处将三对数据做了平均
            uint x1 = (uint)((BitConverter.ToUInt16(re.data, 4) + BitConverter.ToUInt16(re.data, 8) + BitConverter.ToUInt16(re.data, 12)) / 3.0);
            uint x2 = (uint)((BitConverter.ToUInt16(re.data, 6) + BitConverter.ToUInt16(re.data, 10) + BitConverter.ToUInt16(re.data, 14)) / 3.0);
            if (!cal)
                return new KeyValuePair<double, double>(x1, x2);
            else
                return new KeyValuePair<double, double>(PHOTODIODE.cal(x1, r1), PHOTODIODE.cal(x2, r2));
        }

        /// <summary>
        /// 获取/设置C++层平均次数
        /// </summary>
        /// <param name="avg"></param>
        /// <returns></returns>
        public uint GetSetAvg(bool set = false, uint avg = 10)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            if (set)
            {
                to_send.Add(new byte[] { (byte)'A', (byte)'V', (byte)'G', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(avg));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            else
            {
                to_send.Add(new byte[] { (byte)'G', (byte)'A', (byte)'V', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            print_rev(re);
            return BitConverter.ToUInt32(re.data,4);
        }

        /// <summary>
        /// 获取/设置C++层起始阈值
        /// </summary>
        public uint GetSetThr(bool set = false, uint thr = 4000)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            if (set)
            {
                to_send.Add(new byte[] { (byte)'T', (byte)'H', (byte)'R', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(thr));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            else
            {
                to_send.Add(new byte[] { (byte)'G', (byte)'T', (byte)'H', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            print_rev(re);
            return BitConverter.ToUInt32(re.data, 4);
        }

        /// <summary>
        /// 获取/设置C++层比较方式
        /// <para>cmp:</para>
        /// <para>0:相等,1:小于,2:大于</para>
        /// </summary>
        public uint GetSetCmp(bool set = false, uint cmp = 1)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            if (set)
            {
                to_send.Add(new byte[] { (byte)'C', (byte)'M', (byte)'P', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(cmp));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            else
            {
                to_send.Add(new byte[] { (byte)'G', (byte)'C', (byte)'M', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            print_rev(re);
            return BitConverter.ToUInt32(re.data, 4);
        }

        /// <summary>
        /// 获取/设置C++层取样长度（两个Sensor合计）
        /// </summary>
        public uint GetSetLen(bool set = false, uint len = 400)
        {
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            if (set)
            {
                to_send.Add(new byte[] { (byte)'L', (byte)'E', (byte)'N', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(len));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            else
            {
                to_send.Add(new byte[] { (byte)'G', (byte)'L', (byte)'E', (byte)'X' });
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
                to_send.Add(BitConverter.GetBytes(0));
            }
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            print_rev(re);
            return BitConverter.ToUInt32(re.data, 4);
        }

        /// <summary>
        /// 获得上一次C++做好的平均数
        /// </summary>
        /// <returns></returns>
        public List<short> GetLastAvgData()
        {
            if (!connected)
                throw new Exception("服务器未连接");
            List<byte[]> to_send = new List<byte[]>();
            to_send.Add(new byte[] { (byte)'D', (byte)'A', (byte)'T', (byte)'X' });
            to_send.Add(BitConverter.GetBytes(0));
            to_send.Add(BitConverter.GetBytes(0));
            to_send.Add(BitConverter.GetBytes(0));
            var re = c.send_and_receive_sync(Client.byte_connect(to_send));
            print_rev(re);
            List<short> ret = new List<short>();
            var array = re.extra_data.ToArray();
            for (int i = 0; i < re.extra_data.Count / 2; i++)
            {
                ret.Add(BitConverter.ToInt16(array, i * 2));
            }
            return ret;
        }

        /// <summary>
        /// 获得上一次C++做好的平均数并做横向抽点/平均
        /// </summary>
        /// <param name="method">"avg":平均,"samp":抽样</param>
        /// <param name="times">[平均/抽样]的[次数/间隔]</param>
        /// <returns></returns>
        public KeyValuePair<List<short>, List<short>> GetLastAvgDataExtra(string method = "avg", int times = 100)
        {
            var data = GetLastAvgData();
            KeyValuePair<List<short>, List<short>> ret = 
                new KeyValuePair<List<short>, List<short>>(new List<short>(), new List<short>());
            uint temp1 = 0, temp2 = 0;
            for (int i = 0; i < data.Count / 2; i++)
            {
                if (method == "avg")
                {
                    temp1 += (uint)data[i * 2];
                    temp2 += (uint)data[i * 2 + 1];
                }
                if ((i + 1) % times == 0)
                {
                    if (method == "avg")
                    {
                        ret.Key.Add((short)(temp1 / times));
                        ret.Value.Add((short)(temp2 / times));
                        temp1 = 0;
                        temp2 = 0;
                    }
                    else if (method == "samp")
                    {
                        ret.Key.Add(data[i * 2]);
                        ret.Value.Add(data[i * 2 + 1]);
                    }
                }
            }
            return ret;
        }

        Client c = new Client();
        List<Sensor> sensors = new List<Sensor>();
        void print_rev(Client.ReceiveEventArgs arg)
        {
            if (print_received)
            {
                Console.WriteLine(arg.time.ToString());
                Console.WriteLine(Encoding.ASCII.GetString(arg.data, 0, 4) + ":");
                string tmp = "";
                string tmp2;
                tmp += "0x";
                tmp2 = Convert.ToString(BitConverter.ToInt32(arg.data, 4), 16);
                for (int i = 0; i < 8 - tmp2.Length; i++)
                    tmp += "0";
                tmp += tmp2;
                tmp += " 0x";
                tmp2 = Convert.ToString(BitConverter.ToInt32(arg.data, 8), 16);
                for (int i = 0; i < 8 - tmp2.Length; i++)
                    tmp += "0";
                tmp += tmp2;
                tmp += " 0x";
                tmp2 = Convert.ToString(BitConverter.ToInt32(arg.data, 12), 16);
                for (int i = 0; i < 8 - tmp2.Length; i++)
                    tmp += "0";
                tmp += tmp2;
                Console.WriteLine(tmp);
            }
        }
    }
}
