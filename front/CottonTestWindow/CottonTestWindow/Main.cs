using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using CottonTestCoreDynamic;
using System.IO;
using System.Threading;
using System.Drawing.Drawing2D;
using System.Windows.Forms.DataVisualization.Charting;

namespace CottonTestWindowDynamic
{
    public partial class CottonTestWindow : Form
    {
        InterfaceCore core = new InterfaceCore();
        bool recording = false;
        bool temperature_calibration = false;
        string cali_file = "";
        List<double>[] output = new List<double>[3];
        public CottonTestWindow()
        {
            InitializeComponent();
            new ConsoleHelper(this.TextConsole);
            numericUpDown1.Maximum = InterfaceCore.PHOTODIODE.AMP3_RL_STEP_MAX;
            numericUpDown1.Minimum = 0;
            numericUpDown2.Maximum = InterfaceCore.PHOTODIODE.AMP3_RL_STEP_MAX;
            numericUpDown2.Minimum = 0;
            progressBar1.Maximum = (int)InterfaceCore.PHOTODIODE.AD_MAX;
            progressBar2.Maximum = (int)InterfaceCore.PHOTODIODE.AD_MAX;
            core.print_received = checkBox1.Checked;
            for (int i = 0; i < output.Length; i++)
            {
                output[i] = new List<double>();
            }
        }

        private void ButtonConnect_Click(object sender, EventArgs e)
        {
            try
            {
                core.connect(TextIP.Text,int.Parse(TextPort.Text));
                if (core.connected)
                {
                    ButtonStartStop.Enabled = true;
                    buttonReset.Enabled = true;
                }
                else
                {
                    Console.WriteLine("连接失败");
                }
            }
            catch (Exception exc)
            {
                Console.WriteLine(exc.Message);
            }
        }

        private void ButtonStartStop_Click(object sender, EventArgs e)
        {
            try
            {
                if (ButtonStartStop.Text == "开始")
                {
                    TimerData.Interval = (int)uint.Parse(TextFre.Text) * 1000;
                    TimerData.Enabled = true;
                    TimerTemperature.Enabled = true;
                    TextRevserved1.ReadOnly = true;
                    TextRevserved2.ReadOnly = true;
                    TextFre.ReadOnly = true;
                    textBoxSamp.ReadOnly = true;
                    checkBoxSamp.Enabled = false;
                    ButtonStartStop.Text = "停止";
                }
                else
                {
                    TimerData.Enabled = false;
                    TimerTemperature.Enabled = false;
                    TextRevserved1.ReadOnly = false;
                    TextRevserved2.ReadOnly = false;
                    TextFre.ReadOnly = false;
                    textBoxSamp.ReadOnly = false;
                    checkBoxSamp.Enabled = true;
                    ButtonStartStop.Text = "开始";
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void ButtonRead1_Click(object sender, EventArgs e)
        {
            try
            {
                //TextMulti1.Text = core.GetSetRisistor(0).ToString();
                numericUpDown1.Value = core.GetSetRisistor(0);
                checkBoxCool1.Checked = core.GetSetCooler(0);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void ButtonWrite1_Click(object sender, EventArgs e)
        {
            try
            {
                //int x = int.Parse(TextMulti1.Text);
                int x = (int)numericUpDown1.Value;
                if (x > InterfaceCore.PHOTODIODE.AMP3_RL_STEP_MAX || x < 0)
                    throw new Exception("调节超过了16级level");
                core.GetSetRisistor(0, true, x);
                core.GetSetCooler(0, true, checkBoxCool1.Checked);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void ButtonRead2_Click(object sender, EventArgs e)
        {
            try
            {
                //TextMulti2.Text = core.GetSetRisistor(1).ToString();
                numericUpDown2.Value = core.GetSetRisistor(1);
                checkBoxCool2.Checked = core.GetSetCooler(1);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void ButtonWrite2_Click(object sender, EventArgs e)
        {
            try
            {
                //int x = int.Parse(TextMulti2.Text);
                int x = (int)numericUpDown2.Value;
                if (x > InterfaceCore.PHOTODIODE.AMP3_RL_STEP_MAX || x < 0)
                    throw new Exception("调节超过了16级level");
                core.GetSetRisistor(1, true, x);
                core.GetSetCooler(1, true, checkBoxCool2.Checked);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void TimerData_Tick(object sender, EventArgs e)
        {
            try
            {
                string s = "";
                //var data = core.GetLastAvgData();
                //for (int i = 0; i < data.Count / 2; i++)
                //{
                //    s += data[i * 2].ToString() + "," + data[i * 2 + 1].ToString() + "," +
                //        ((int)data[i * 2] - (int)data[i * 2 + 1]).ToString() + "\r\n";
                //}
                var data2 = core.GetLastAvgDataExtra(checkBoxSamp.Checked ? "avg" : "samp", int.Parse(textBoxSamp.Text));
                for (int i = 0; i < data2.Value.Count; i++)
                {
                    s += data2.Key[i].ToString() + "," + data2.Value[i].ToString() + "," +
                        ((int)data2.Key[i] - (int)data2.Value[i]).ToString() + "\r\n";
                }
                textBoxInfo.Text = s;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void TimerTemperature_Tick(object sender, EventArgs e)
        {
            try
            {
                if (temperature_calibration)
                {
                    double val = core.GetTemperature(0, int.Parse(TextRevserved1.Text));
                    TextTemp1.Text = Math.Round(val, 2).ToString();
                    val = core.GetTemperature(1, int.Parse(TextRevserved2.Text));
                    TextTemp2.Text = Math.Round(val, 2).ToString();
                }
                else
                {
                    double val = core.GetTemperature(0);
                    TextTemp1.Text = Math.Round(val, 2).ToString();
                    val = core.GetTemperature(1);
                    TextTemp2.Text = Math.Round(val, 2).ToString();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            try
            {
                try
                {
                    core.GetSetPeriod(true);
                    core.GetSetWidth(true);
                    ButtonRead1_Click(this, new EventArgs());
                    ButtonRead2_Click(this, new EventArgs());
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                openFileDialog1.Filter = "文本文件 (*.txt)|*.txt|All files (*.*)|*.*";
                openFileDialog1.InitialDirectory = System.Environment.CurrentDirectory;
                var res = openFileDialog1.ShowDialog();
                if (res == DialogResult.OK)
                {
                    core.read_config(openFileDialog1.FileName);
                    temperature_calibration = true;
                    cali_file = openFileDialog1.FileName;
                    Console.WriteLine("配置文件读取成功，将按照指定的传感器编号执行温度校正");
                }
                else
                {
                    Console.WriteLine("未选择温度校正配置文件，将按照正常计算流程估算温度");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            core.print_received = checkBox1.Checked;
        }

        private void buttonPrint_Click(object sender, EventArgs e)
        {
            Bitmap bit = new Bitmap(this.Width, this.Height);//实例化一个和窗体一样大的bitmap
            Graphics g = Graphics.FromImage(bit);
            g.CompositingQuality = CompositingQuality.HighQuality;//质量设为最高
            g.CopyFromScreen(this.Left, this.Top, 0, 0, new Size(this.Width, this.Height));//保存整个窗体为图片
            //只保存某个控件（这里是panel游戏区）
            //g.CopyFromScreen(panel游戏区 .PointToScreen(Point.Empty), Point.Empty, panel游戏区.Size);
            string str_dir = System.Environment.CurrentDirectory + "/screen_shot";
            if (Directory.Exists(str_dir) == false)//如果不存在就创建file文件夹
            {
                Directory.CreateDirectory(str_dir);
            }
            bit.Save(str_dir+"/window_" +DateTime.Now.ToString("yy_MM_dd_hh_mm_ss")+".png");//默认保存格式为PNG，保存成jpg格式质量不是很好
        }

        private void button_avg_write_Click(object sender, EventArgs e)
        {
            try
            {
                textBoxAVG.Text = core.GetSetAvg(true ,uint.Parse(textBoxAVG.Text)).ToString();
                textBoxTHR.Text = core.GetSetThr(true, uint.Parse(textBoxTHR.Text)).ToString();
                textBoxCMP.Text = core.GetSetCmp(true, uint.Parse(textBoxCMP.Text)).ToString();
                textBoxLEN.Text = core.GetSetLen(true, uint.Parse(textBoxLEN.Text)).ToString();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void button_avg_read_Click(object sender, EventArgs e)
        {
            try
            {
                textBoxAVG.Text = core.GetSetAvg().ToString();
                textBoxTHR.Text = core.GetSetThr().ToString();
                textBoxCMP.Text = core.GetSetCmp().ToString();
                textBoxLEN.Text = core.GetSetLen().ToString();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void button1_MouseDown(object sender, MouseEventArgs e)
        {
            try
            {
                for (int i = 0; i < output.Length; i++)
                {
                    output[i].Clear();
                }
                recording = true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void button1_MouseUp(object sender, MouseEventArgs e)
        {
            try
            {
                for (int i = 0; i < output.Length; i++)
                {
                    if (output[i].Count == 0)
                    {
                        Console.WriteLine("按键时间太短或未收到任何数据，本次平均无效");
                        textBoxInfo.AppendText("按键时间太短或未收到任何数据，本次平均无效\r\n");
                        break;
                    }
                    else
                    {
                        var x = output[i].Sum() / output[i].Count;
                        textBoxInfo.AppendText(Math.Round(x).ToString() + ",");
                    }
                    
                }
                for (int i = 0; i < output.Length; i++)
                {
                    output[i].Clear();
                }
                textBoxInfo.AppendText("\r\n");
                recording = false;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                string str_dir = System.Environment.CurrentDirectory + "/data/"+ DateTime.Now.ToString("yy_MM_dd_hh_mm_ss");
                if (Directory.Exists(str_dir) == false)//如果不存在就创建file文件夹
                {
                    Directory.CreateDirectory(str_dir);
                }

                FileStream fs = new FileStream(str_dir + "/data_" + DateTime.Now.ToString("yy_MM_dd_hh_mm_ss") + ".csv", FileMode.Create);
                StreamWriter sw = new StreamWriter(fs);
                //开始写入
                sw.Write(textBoxInfo.Text);
                //清空缓冲区
                sw.Flush();
                //关闭流
                sw.Close();
                fs.Close();
                Bitmap bit = new Bitmap(this.Width, this.Height);
                Graphics g = Graphics.FromImage(bit);
                g.CompositingQuality = CompositingQuality.HighQuality;
                g.CopyFromScreen(this.Left, this.Top, 0, 0, new Size(this.Width, this.Height));
                bit.Save(str_dir + "/window_" + DateTime.Now.ToString("yy_MM_dd_hh_mm_ss") + ".png");
                InfoForm infoform = new InfoForm(str_dir);
                if (temperature_calibration)
                    infoform.set_num(TextRevserved1.Text, TextRevserved2.Text);
                infoform.ShowDialog();
                if (temperature_calibration)
                {
                    System.IO.File.Copy(cali_file, str_dir + "/" + Path.GetFileName(cali_file), true);
                }
                Console.WriteLine("实验信息已经保存到" + str_dir);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void textBoxInfo_TextChanged(object sender, EventArgs e)
        {
            try
            {
                string[] sp_line = textBoxInfo.Text.Split(new char[] { '\r', '\n' });
                chart1.Series.Clear();
                chart2.Series.Clear();
                Series x1, x2, x3;
                x1 = new Series("Sensor1");
                x2 = new Series("Sensor2");
                x3 = new Series("Sensor1 - Sensor2");
                int count = 0;
                for (int i = 0; i < sp_line.Length; i++)
                {
                    try
                    {
                        string[] line = sp_line[i].Split(new char[] { ',' });
                        if (line.Length >= 3 && line[0].Length > 0 && line[1].Length > 0 && line[2].Length > 0)
                        {
                            x1.Points.AddXY(count, double.Parse(line[0]));
                            x2.Points.AddXY(count, double.Parse(line[1]));
                            x3.Points.AddXY(count, double.Parse(line[2]));
                            count++;
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                    }
                }
                x1.Color = Color.Red;
                x2.Color = Color.Blue;
                x3.Color = Color.Green;
                x1.ChartType = SeriesChartType.FastLine;
                x2.ChartType = SeriesChartType.FastLine;
                x3.ChartType = SeriesChartType.FastLine;
                chart1.Series.Add(x1);
                chart1.Series.Add(x2);
                chart2.Series.Add(x3);
                chart1.ChartAreas[0].AxisX.MajorGrid.Enabled = false;
                chart1.ChartAreas[0].AxisY.MajorGrid.Enabled = false;
                chart2.ChartAreas[0].AxisX.MajorGrid.Enabled = false;
                chart2.ChartAreas[0].AxisY.MajorGrid.Enabled = false;
                
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {
                core.generate_sample_config();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            var re = MessageBox.Show("确定清空数据吗？", "清空确认", MessageBoxButtons.YesNo);
            if (re == DialogResult.Yes)
            {
                textBoxInfo.Clear();
            }
        }
    }

    public class ConsoleHelper : TextWriter
    {

        int count = 0;

        private System.Windows.Forms.TextBox _textBox { set; get; }//如果是wpf的也可以换做wpf的输入框

        public ConsoleHelper(System.Windows.Forms.TextBox textBox)
        {
            this._textBox = textBox;
            Console.SetOut(this);
        }

        public override void Write(string value)
        {
            if (_textBox.IsHandleCreated)
                _textBox.BeginInvoke(new ThreadStart(() =>
                {
                    _textBox.AppendText("[" + count.ToString() + "]" + value + " ");
                    count++;
                }));
        }

        public override void WriteLine(string value)
        {
            if (_textBox.IsHandleCreated)
                _textBox.BeginInvoke(new ThreadStart(() =>
                {
                    _textBox.AppendText("[" + count.ToString() + "]" + value + "\r\n");
                    count++;
                }));
        }

        public override Encoding Encoding//这里要注意,重写wirte必须也要重写编码类型
        {
            get { return Encoding.UTF8; }
        }


    }
}
