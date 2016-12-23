using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CottonTestWindowDynamic
{
    public partial class InfoForm : Form
    {
        string str;
        public InfoForm(string _str)
        {
            InitializeComponent();
            str = _str;
        }

        private void ButtonConnect_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void InfoForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            Bitmap bit = new Bitmap(this.Width, this.Height);
            Graphics g = Graphics.FromImage(bit);
            g.CompositingQuality = CompositingQuality.HighQuality;
            g.CopyFromScreen(this.Left, this.Top, 0, 0, new Size(this.Width, this.Height));
            bit.Save(str + "/info_window_" + DateTime.Now.ToString("yy_MM_dd_hh_mm_ss") + ".png");
        }

        public void set_num(string a, string b)
        {
            textBox3.Text = a;
            textBox5.Text = b;
        }
    }
}
