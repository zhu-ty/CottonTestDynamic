using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CottonTestCoreDynamic
{
    class Program
    {
        static void Main(string[] args)
        {
            Client c = new Client();
            while (true)
            {
                try
                {
                    string command = Console.ReadLine();
                    if (command == "e")
                        break;
                    string[] split = command.Split(new char[] { ' ' });
                    if (split[0] == "c")
                    {
                        System.Net.IPAddress ip = System.Net.IPAddress.Parse(split[1]);
                        c.connect(ip);
                    }
                    if (split[0] == "g")
                    {
                        List<byte[]> to_send = new List<byte[]>();
                        to_send.Add(new byte[] { (byte)'G', (byte)'E', (byte)'T', (byte)'X' });
                        to_send.Add(BitConverter.GetBytes(0xff01));
                        to_send.Add(BitConverter.GetBytes(0));
                        to_send.Add(BitConverter.GetBytes(0));
                        var rev = c.send_and_receive_sync(Client.byte_connect(to_send));
                        print_rev(rev);
                    }
                    if (split[0] == "s")
                    {
                        int x = int.Parse(split[1]);
                        List<byte[]> to_send = new List<byte[]>();
                        to_send.Add(new byte[] { (byte)'S', (byte)'E', (byte)'T', (byte)'X' });
                        to_send.Add(BitConverter.GetBytes(0xff01));
                        to_send.Add(BitConverter.GetBytes(x));
                        to_send.Add(BitConverter.GetBytes(0));
                        var rev = c.send_and_receive_sync(Client.byte_connect(to_send));
                        print_rev(rev);
                    }
                    if (split[0] == "gx")
                    {
                        List<byte[]> to_send = new List<byte[]>();
                        to_send.Add(new byte[] { (byte)'R', (byte)'A', (byte)'W', (byte)'X' });
                        to_send.Add(BitConverter.GetBytes(0));
                        to_send.Add(BitConverter.GetBytes(0));
                        to_send.Add(BitConverter.GetBytes(0));
                        var rev = c.send_and_receive_sync(Client.byte_connect(to_send));
                        print_rev(rev);
                    }
                    if (split[0] == "pt_test")
                    {
                        double[] x2 = new double[2];
                        double[] y2 = new double[2];
                        x2[0] = double.Parse(split[1]);
                        x2[1] = double.Parse(split[2]);
                        y2[0] = double.Parse(split[3]);
                        y2[1] = double.Parse(split[4]);
                        var lsret = LeastSquare.MultiLine(x2, y2, 2, 1);
                        Console.WriteLine(lsret[0].ToString() + "  " + lsret[1].ToString());
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }
        }

        static void print_rev(Client.ReceiveEventArgs arg)
        {
            Console.WriteLine(arg.time.ToString());
            Console.WriteLine(Encoding.ASCII.GetString(arg.data,0,4) + ":");
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
