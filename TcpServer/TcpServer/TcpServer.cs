using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Collections;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

namespace TcpServer
{
    public partial class TcpServer : Form
    {
                                
        public TcpServer()                     
        {
            InitializeComponent();
            textBox1.Text = "192.168.43.38";
            textBox2.Text = "8080";
          //  textBox1.Text = GetIP();
            ListBox.CheckForIllegalCrossThreadCalls = false;
        }
        
        public Thread mythread1, mythread2, mythread4;
        private bool bstar = true;     //判断是否开启服务器
        private static string ip;      //当前连接的ip地址
        private static int port;
        private static Socket socket;
        private Socket _socket;
        private static Dictionary<string, Socket> dicOnline;
        private Socket _tempSocket;                   //当前连接套接字
        private ArrayList PoolInfoMsg;
        private ArrayList conected_client = new ArrayList();
        public delegate void UpdateClientListCallback();
        private int m_clientCount = 0;
        private System.Collections.ArrayList m_workerSocketList = ArrayList.Synchronized(new System.Collections.ArrayList());
        public static Dictionary<string, Socket> listSocket = new Dictionary<string, Socket>();
        private enum DataMode { Text, Hex }
        private bool Keyjudge = false;  //用于键盘控制小车
        private bool resetB = false;

        private byte work_mode = 0;                 //系统工作模式
        /// <summary>
        /// 0 ： 系统刚启动，等待命令
        /// 1 ： 小车路径探测
        /// 2 ： 找到信息，开启小车路径规划
        /// 3 ： 小车1故障，开启小车寻回
        /// 4 ： 小车2故障         需要重启系统
        /// </summary>


        [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        public struct route_data                                     //路径规划结构体
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
            public byte[] card_flag;                                //卡标号

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 12)]
            public byte[] card_id;                                  //id值

            public uint lastcard_num ;                               //第一次读卡时车来的方向
        }
        route_data[] card_data = new route_data[26];               //矩阵数组1--25使用
        byte[] Aim_card = new byte[6];                   //目标Rfid卡号


        //String GetIP()                                 //获取主机IP
        //{
        //    String HostName = Dns.GetHostName();
        //    IPHostEntry iphostentry = Dns.GetHostByName(HostName);
        //    String IPStr = "";
        //    foreach (IPAddress ipaddress in iphostentry.AddressList)
        //    {
        //        IPStr = ipaddress.ToString();
        //        return IPStr;
        //    }
        //    return IPStr;
        //}

        private void button1_Click(object sender, EventArgs e)          //开启TCP服务器
        {
             if (bstar)
            {
                SocketAdmin(this.textBox1.Text.Trim(), out _socket, Convert.ToInt32(this.textBox2.Text.Trim()), out dicOnline);
                mythread2 = new Thread(LoopInfoMsg);//循环发送消息的方法
                mythread2.IsBackground = true;
                mythread2.Start();
                bstar = false;
                button1.Text = "关闭";
                this.KeyPreview = true;
            }
            else
            {
                try
                {
                    list.Items.Clear();
                    listSocket.Clear();
                    bstar = true;
                    CloseSockets();
                    button1.Text = "开始";
                    this.KeyPreview = false;
                }
                catch (Exception)
                {

                }
            }
        }

        public void LoopInfoMsg()                    //具体传送过程
        {
 //           Is_busy = true;
            while (_socket != null)
            {
                try
                {
                    if (PoolInfoMsg != null && PoolInfoMsg.Count > 0)//表示有消息
                    {
                        _tempSocket.Send((byte[])PoolInfoMsg[0]);
                        PoolInfoMsg.RemoveAt(0);//移除已发的消息
                    }
                    Thread.Sleep(100);

                }
                catch (Exception)
                {
                    //  MessageBox.Show(ex.Message);
                }
            }
 //           Is_busy = false;
        }

        bool Movearr = false;                      //是否重复连接标志，同一ip
        void UpdateClientList()                   //更新连接的client
        {
            try
            {
                
                if (list.Items.Count != listSocket.Count)  //说明有新的用户连上//是否刷新多个用户的信息？
                {
                    if (conected_client == null)
                    {
                        conected_client = new ArrayList();
                    }
                    else conected_client.Clear();

                    foreach (var item in listSocket.Keys)
                    {
                        conected_client.Add(item);                 
                    }
                    for(int jarr=0; jarr < conected_client.Count-1; jarr++)
                    {
                        for(int iarr=jarr+1; iarr< conected_client.Count; iarr++)
                        {
                            RemoveUselessArray((string)conected_client[jarr], (string)conected_client[iarr]);
                            if (Movearr == true)            //存在多次连接情况,删除无用连接
                            {
                                listSocket.Remove((string)conected_client[jarr]);
                                conected_client.RemoveAt(jarr);
                                textBox3.Text = (string)conected_client[jarr];
                                Movearr = false;
                            }
                        }      
                    }
                    conected_client.Sort();                   //对连接ip排序
                    list.Items.Clear();
                    foreach (var item in listSocket.Keys)
                    {
                        list.Items.Add(item);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        void RemoveUselessArray(System.String firarr, System.String secarr)      //判断ip地址是否相同
        {
            string[] Sarray1 = firarr.Split(':');
            string[] Sarray2 = secarr.Split(':');

            if (Sarray1[0].CompareTo(Sarray2[0]) == 0)
                Movearr = true;
        }

        private void UpdateClientListControl()
        {
            if (InvokeRequired)  //invoke方法的初衷是为了解决在某个非某个控件创建的线程中刷新该控件可能会引发异常的问题。说的可能比较拗口，举个例子：主线程中存在一个文本控件，在一个子线程中要改变该文本的值，此时会有可能引发异常。
                                 //为了避免该问题，需要在子线程中使用invoke方法来封装刷新文本内容的函数。Invoke 或者 BeginInvoke 去调用，两者的区别就是Invoke 会导致工作线程等待，而BeginInvoke 则不会
            {
                list.BeginInvoke(new UpdateClientListCallback(UpdateClientList), null);
            }
            else
            {
                UpdateClientList();
            }
        }

        private void SocketAdmin(string _ip, out Socket _socket, int _port, out Dictionary<string, Socket> _dicSocket)
        {
            ip = _ip;
            port = _port;
            ServerRun();
            _socket = socket;
            _dicSocket = listSocket;
        }

        private void ServerRun()           //建立套接字以连接新的请求对象
        {
            try
            {
                socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);     //创建一个新的套接字
                IPEndPoint _ipport = new IPEndPoint(IPAddress.Parse(ip), port);
                socket.Bind(_ipport);
                socket.Listen(50);
                socket.BeginAccept(new AsyncCallback(DoAcceptTcpClient), socket);
            }
            catch (Exception ex)              //处理程序的异常
            {
                MessageBox.Show(ex.Message);
            }

        }
       
        public void DoAcceptTcpClient(IAsyncResult ar)
        {
            try
            {
                Socket workerSocket = socket.EndAccept(ar);          //创建一个新的socket进行主机通信
                Interlocked.Increment(ref m_clientCount);

                m_workerSocketList.Add(workerSocket);                  //正在工作的套接字加入到worklist中
                ip = workerSocket.RemoteEndPoint.ToString();

                if (!listSocket.ContainsKey(ip))                     //判断listsocket中是否包含指点键
                    listSocket.Add(ip, workerSocket);                //如果新的连接，将值存入listSocket中
                    UpdateClientListControl();
                    WaitForData(workerSocket, m_clientCount);
                    socket.BeginAccept(new AsyncCallback(DoAcceptTcpClient), null);
            }
            catch (Exception)
            {

            }
        }
        public AsyncCallback pfnWorkerCallBack;

        // Start waiting for data from the client
        public void WaitForData(System.Net.Sockets.Socket soc, int clientNumber)          //等待已连接client发送数据
        {
            try
            {
                if (pfnWorkerCallBack == null)
                {
                    pfnWorkerCallBack = new AsyncCallback(OnDataReceived);             //接收信息后的处理
                }
                SocketPacket theSocPkt = new SocketPacket(soc, clientNumber);

                soc.BeginReceive(theSocPkt.dataBuffer, 0, theSocPkt.dataBuffer.Length, SocketFlags.None, pfnWorkerCallBack, theSocPkt);
            }
            catch (SocketException se)
            {
                MessageBox.Show(se.Message);
            }
        }
        string stringdata;

        public class SocketPacket                    //储存当前socket和clientNumber的结构体
        {
            // Constructor which takes a Socket and a client number
            public SocketPacket(System.Net.Sockets.Socket socket, int clientNumber)
            {
                m_currentSocket = socket;
                m_clientNumber = clientNumber;
            }
            public System.Net.Sockets.Socket m_currentSocket;
            public int m_clientNumber;
            // Buffer to store the data sent by the client
            public byte[] dataBuffer = new byte[1024];         //接受的client数据
        }

        private void Auto_sendmes(byte[] byteArr)    //自动发送消息过程
        {
            if (conected_client.Count < 1)
                MessageBox.Show("当前无可用连接");
            else
            {
                if (conected_client.Count < 2)
                {
                    _tempSocket = dicOnline[conected_client[0].ToString()];
                }
                else
                {
                    if (work_mode == 1 || work_mode == 0)         //车一正常或系统刚启动
                    {
                        _tempSocket = dicOnline[conected_client[0].ToString()];
                    }
                    else                             //车一出错或者完成任务
                    {
                        _tempSocket = dicOnline[conected_client[1].ToString()];
                    }
                }
            }
            if (PoolInfoMsg == null)
            {
                PoolInfoMsg = new ArrayList();
            }
            PoolInfoMsg.Add(byteArr);              //将需要发送的信息存入poolInfoMsg中       

        }
        private bool BytesCompare_Step(byte[] b1, byte[] b2)   //字符串数组比较函数
        {
            if (b1 == null || b2 == null) return false;
            if (b1.Length != b2.Length) return false;
            for (int i = 0; i < b1.Length; ++i)
            {
                if (b1[i] != b2[i]) return false;
            }
            return true;
        }

        private void Marx_border(Int16 i)                    //边界限定函数
        {
            if (i < max_cardnum + 1)
            {
                card_data[i].card_flag[0] |= nomove_flag[4];    //西边界
            }
            else
            {
                if (i > max_cardnum * (max_cardnum - 1))
                    card_data[i].card_flag[0] |= nomove_flag[2];  //东边界
            }
            if (i % max_cardnum == 1)
            {
                card_data[i].card_flag[0] |= nomove_flag[3];   //南边界
            }
            else
            {
                if (i % max_cardnum == 0)
                    card_data[i].card_flag[0] |= nomove_flag[1];  //北边界
            }
        }
        
        private void Search_route(uint i, uint line)
        {
            if (line % 2 == 1) //优先往北走，区分南北，为了尽快建立卡矩阵模型
            {
                if ((card_data[i].card_flag[0] & nomove_flag[1]) == 0)
                {
                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n")); //北走
                }
                else
                {
                    if ((card_data[i].card_flag[0] & nomove_flag[2]) == 0)
                    {
                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n"));//东走
                    }
                    else
                    {
                        if ((card_data[i].card_flag[0] & nomove_flag[3]) == 0)
                        {
                            Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U5#\r\n")); //南走
                        }
                        else
                        {
                            if ((card_data[i].card_flag[0] & nomove_flag[4]) == 0)
                            {
                                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*UA#\r\n")); //西走
                            }
                        }
                    }
                }
            }
            else          //优先往南走
            {
                if ((card_data[i].card_flag[0] & nomove_flag[3]) == 0)
                {
                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U5#\r\n")); //南走
                }
                else
                {
                    if ((card_data[i].card_flag[0] & nomove_flag[2]) == 0)
                    {
                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n"));//东走
                    }
                    else
                    {
                        if ((card_data[i].card_flag[0] & nomove_flag[1]) == 0)
                        {
                            Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n")); //北走
                        }
                        else
                        {
                            if ((card_data[i].card_flag[0] & nomove_flag[4]) == 0)
                            {
                                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*UA#\r\n")); //西走
                            }
                        }
                    }
                }

            }
        }
        private void Search_vanroute(uint i, uint line)          //优先走没走过的卡片
        {
            if (((card_data[i].card_flag[0] & yesmove_flag[1]) == 0) && ((card_data[i].card_flag[0] & nomove_flag[1]) == 0))    //方向能走且没有走过
            {
                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n")); //北走
            }
            else
            {
                if (((card_data[i].card_flag[0] & yesmove_flag[2]) == 0) && ((card_data[i].card_flag[0] & nomove_flag[2]) == 0))
                {
                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n"));//东走
                }
                else
                {
                    if (((card_data[i].card_flag[0] & yesmove_flag[3]) == 0) && ((card_data[i].card_flag[0] & nomove_flag[3]) == 0))
                    {
                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U5#\r\n")); //南走
                    }
                    else
                    {
                        if ((card_data[i].card_flag[0] & yesmove_flag[4]) == 0 && ((card_data[i].card_flag[0] & nomove_flag[4]) == 0))
                        {
                            Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*UA#\r\n")); //西走
                        }
                        else                                 //都走过，返回上一张卡，再处理
                        {
                            if(i > card_data[i].lastcard_num)
                            switch (i - card_data[i].lastcard_num)
                            {
                                case 1:
                                    {
                                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U5#\r\n")); //南走
                                        break;
                                    }
                                case 3:
                                    {
                                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*UA#\r\n")); //西走
                                        break;
                                    }
                            }
                            else
                                switch (card_data[i].lastcard_num - i)
                                {
                                    case 1:
                                        {
                                            Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n")); //北走
                                            break;
                                        }
                                    case 3:
                                        {
                                            Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n")); //东走
                                            break;
                                        }
                                }

                        }
                    }
                }
            }
        }
        private void Error_detach(string szData)
        {
            switch (work_mode)
            {
                case 0:
                    {
                        if (szData.CompareTo("Aconnected") == 0)
                        {
                            pictureBox1.Visible = true;
                            pictureBox5.Visible = false;
                        }

                        if (szData.CompareTo("Bconnected") == 0)
                        {
                            resetB = true;
                            pictureBox2.Visible = true;
                            pictureBox6.Visible = false;
                        }
                        break;
                    }
                case 1:
                    {
                        if (szData.CompareTo("B1") == 0)
                        {
                            Delay(1);
    //                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n"));
                            string mesg2 = "/r/n" + "There is barrier";
                            OnUpdateRichEdit(mesg2);
                            card_data[last_i].card_flag[0] |= nomove_flag[1];
                            if (last_i + 1 < 27)
                                card_data[last_i + 1].card_flag[0] |= nomove_flag[3];
                            Search_vanroute(card_seq, line_num);   //优先搜索未走路线
                            Delay(8);
                            Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U1#\r\n"));
                        }
                        else
                        {
                            if (szData.CompareTo("B2") == 0)
                            {
                                Delay(1);
                                string mesg2 = "/r/n" + "There is barrier";
                                OnUpdateRichEdit(mesg2);
                                card_data[last_i].card_flag[0] |= nomove_flag[2];
                                if (last_i + max_cardnum < 27)
                                    card_data[last_i + max_cardnum].card_flag[0] |= nomove_flag[4];
                                if(last_i == max_cardnum+1 )      //判断是否特殊情况
                                {
                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n"));
                                }
                                else Search_vanroute(card_seq, line_num);   //优先搜索未走路线
                                Delay(8);
                                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U1#\r\n"));
                            }
                            else
                            {
                                if (szData.CompareTo("B3") == 0)
                                {
                                    Delay(1);
                                    string mesg2 = "/r/n" + "There is barrier";
                                    OnUpdateRichEdit(mesg2);
                                    card_data[last_i].card_flag[0] |= nomove_flag[3];
                                    if (last_i - 1 > 0 && last_i - 1 < 27)
                                        card_data[last_i - 1].card_flag[0] |= nomove_flag[1];
                                    Search_vanroute(card_seq, line_num);   //优先搜索未走路线
                                    Delay(8);
                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U1#\r\n"));
                                }
                                else
                                {
                                    if (szData.CompareTo("B4") == 0)
                                    {
                                        Delay(1);
                                        string mesg2 = "/r/n" + "There is barrier";
                                        OnUpdateRichEdit(mesg2);
                                        card_data[last_i].card_flag[0] |= nomove_flag[4];
                                        if (last_i - max_cardnum > 0 && last_i - max_cardnum < 27)
                                            card_data[last_i - max_cardnum].card_flag[0] |= nomove_flag[2];
                                        Search_vanroute(card_seq, line_num);   //优先搜索未走路线
                                        Delay(8);
                                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U1#\r\n"));
                                    }
                                    else if (szData.CompareTo("B5") == 0)
                                    {
                                        Delay(1);
                                        string mesg2 = "/r/n" + "There is barrier";
                                        OnUpdateRichEdit(mesg2);
                                        Search_vanroute(card_seq, line_num);   //优先搜索未走路线
                                        Delay(8);
                                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U1#\r\n"));
                                    }
                                }
                            }
                        }
                        break;
                    }
            }

            if (szData.CompareTo("*****") == 0)
            {
                work_mode = 3;                         //故障出现
                string mesg1 = "/r/n" + "CarA error! Start CarB !";
                OnUpdateRichEdit(mesg1);
                pictureBox1.Visible = false;
                pictureBox5.Visible = true;

            }
            else
            {
                if (szData.CompareTo("success") == 0)
                {
                    work_mode = 2;
                    string mesg2 = "/r/n" + "Find the location! Start CarB !";
                    OnUpdateRichEdit(mesg2);
                }
                else
                {
                    if (szData.CompareTo("#####") == 0)               //故障排除标志
                        work_mode = 0;
                }
            }


        }
        private void Sequence(uint i)                          //排序列
        {
            switch (i)
            {
                case 1:
                    card_seq = card_seq + 1;
                    break;
                case 2:
                    card_seq = card_seq + max_cardnum;
                    break;
                case 3:
                    card_seq = card_seq - 1;
                    break;
                case 4:
                    card_seq = card_seq - max_cardnum;
                    break;

            }
        }

        //******************************************* 路径规划函数****************************************//
        uint[] route = new uint[25];   //已规划路径缓存区
        private void Route_plan()
        {
            byte flag;
            bool suc = true, half_suc, way_ok = false;
            int x ;
            byte k = 0, j;
            uint i, la_i, aim_num;
            uint[,] ma = new uint[16, 15];

            MessageBox.Show("进入路径规划！");
            //**********初始化规划矩阵********************//
            x = (int)max_cardnum * (int)max_cardnum;
            //for (j = 0; j < x; j++)   //初始化矩阵
            //    for (k = 0; k < 15; k++)
            //        ma[j, k] = 0;

            for (j = 0; j < x; j++)  //初始化可行道路，默认都可以走
                for (k = 1; k < 5; k++)
                    ma[j, k] = 1;

            for (j = 0; j < x; j++)     //初始化路径值
                for (k = 5; k < 9; k++)
                    ma[j, k] = 50;

            for (j = 0; j < x; j++)    //初始化权值， 默认都未知
                for (k = 9; k < 13; k++)
                    ma[j, k] = 4;

            for (j = 0; j < x; j++)         //禁止通行道路初始化
            {
                flag = card_data[j + 1].card_flag[0];
                if ((flag & nomove_flag[1]) == nomove_flag[1])                      //北不能走
                {
                    ma[j, 1] = 0;
                }
                if ((flag & nomove_flag[2]) == nomove_flag[2])                                     //东不能走
                {
                    ma[j, 2] = 0;
                }
                if ((flag & nomove_flag[3]) == nomove_flag[3])                     //南不能走
                {
                    ma[j, 3] = 0;
                }
                if ((flag & nomove_flag[4]) == nomove_flag[4])                                      //西不能走
                {
                    ma[j, 4] = 0;
                }
            }

            for (j = 0; j < x; j++)      //已走道路初始化
            {
                flag = card_data[j + 1].card_flag[0];
                if ((flag & yesmove_flag[1]) == yesmove_flag[1])                      //北已走过
                {
                    ma[j, 9] = 1;
                }
                if ((flag & yesmove_flag[2]) == yesmove_flag[2])                                     //东已走过
                {
                    ma[j, 10] = 1;
                }
                if ((flag & yesmove_flag[3]) == yesmove_flag[3])                     //南已走过
                {
                    ma[j, 11] = 1;
                }
                if ((flag & yesmove_flag[4]) == yesmove_flag[4])                                      //西已走过
                {
                    ma[j, 12] = 1;
                }
            }
            aim_num = endpoint[1];     //目标号
            ma[aim_num - 1, 0] = 1;

            while (suc)
            {
                i = aim_num-1;
                half_suc = true;
                while (half_suc)             //路径规划具体过程
                {
                    la_i = i;
                    if (ma[i, 1] == 1 || ma[i, 2] == 1 || ma[i, 3] == 1 || ma[i, 4] == 1)
                    {
                        if ((ma[i, 4] == 1))
                        {
                            ma[i - max_cardnum, 6] = ma[i, 0] + ma[i, 12];
                            ma[i, 4] = 2;
                            i = i - max_cardnum;
                            ma[i, 2] = 3;
                        }
                        else
                        {
                            if ((ma[i, 3] == 1) && (ma[i - 1, 14] != 4))
                            {
                                ma[i - 1, 5] = ma[i, 0] + ma[i, 11];
                                ma[i, 3] = 2;
                                i = i - 1;
                                ma[i, 1] = 3;
                            }
                            else
                            {
                                if ((ma[i, 2] == 1) && (ma[i + max_cardnum, 14] != 4))
                                {
                                    ma[i + max_cardnum, 8] = ma[i, 0] + ma[i, 10];
                                    ma[i, 2] = 2;
                                    i = i + max_cardnum;
                                    ma[i, 4] = 3;
                                }
                                else
                                {
                                    if ((ma[i, 1] == 1) && (ma[i + 1, 14] != 4))
                                    {
                                        ma[i + 1, 7] = ma[i, 0] + ma[i, 9];
                                        ma[i, 1] = 2;
                                        i = i + 1;
                                        ma[i, 3] = 3;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if ((ma[i, 4] == 3))
                        {
                            ma[i - max_cardnum, 6] = ma[i, 0] + ma[i, 12];
                            ma[i, 4] = 2;
                            i = i - max_cardnum;
                            if (ma[i, 2] != 2)
                                ma[i, 2] = 3;
                        }
                        else
                        {
                            if ((ma[i, 3] == 3) && (ma[i - 1, 14] != 4))
                            {
                                ma[i - 1, 5] = ma[i, 0] + ma[i, 11];
                                ma[i, 3] = 2;
                                i = i - 1;
                                if (ma[i, 1] != 2)
                                    ma[i, 1] = 3;
                            }
                            else
                            {
                                if ((ma[i, 2] == 3) && (ma[i + max_cardnum, 14] != 4))
                                {
                                    ma[i + max_cardnum, 8] = ma[i, 0] + ma[i, 10];
                                    ma[i, 2] = 2;
                                    i = i + max_cardnum;
                                    if (ma[i, 4] != 2)
                                        ma[i, 4] = 3;
                                }
                                else
                                {
                                    if ((ma[i, 1] == 3) && (ma[i + 1, 14] != 4))
                                    {
                                        ma[i + 1, 7] = ma[i, 0] + ma[i, 9];
                                        ma[i, 1] = 2;
                                        i = i + 1;
                                        if (ma[i, 3] != 2)
                                            ma[i, 3] = 3;
                                    }
                                    else
                                    {
                                        if (way_ok)
                                        {
                                            half_suc = false;
                                            suc = false;
                                        }
                                        else
                                        {
                                            MessageBox.Show("路径规划错误");
                                            return;
                                        }
                                    
                                    }
                                }
                            }
                        }

                    }

                    if (i != aim_num)
                    {
                        ma[i, 0] = ma[i, 5];
                        k = 5;
                    }

                    for (j = 5; j < 9; j++)
                    {                     //biggest value store to ma[i][0]
                        if (ma[i, j] < ma[i, 0])
                        {
                            ma[i, 0] = ma[i, j];
                            k = j;
                        }
                    }

                    switch (k - 4)
                    {
                        case 1:
                            ma[i, 13] = i + 1;
                            break;
                        case 2:
                            ma[i, 13] = i + 3;
                            break;
                        case 3:
                            ma[i, 13] = i - 1;
                            break;
                        case 4:
                            ma[i, 13] = i - 3;
                            break;
                    }
                    ma[la_i, 14] += 1;

                    if (i == 0 || i == aim_num)
                        half_suc = false;
                    if (i == 0)    //找到目标
                        way_ok = true;
                }

                k = 0;
                for (j = 0; j < 9; j++)       //判断规划是否完成
                    if (ma[j, 14] != 0)
                    {
                        k++;
                    }
                if (k == 8)
                    suc = false;
            }

            string mesg;
            for (j = 0; j < x; j++)
            {
                for (k = 0; k < 15; k++)
                {
                    mesg = " " + ma[j, k].ToString();
                    textBox1_receive.Text += mesg;
                }
                textBox1_receive.Text += Environment.NewLine;
            }

            for(j=1; j<26; j++)         //打印卡片信息
            {
                stringdata = BitConverter.ToString(card_data[j].card_id);
                string mesgg = "\r\n" + stringdata;
                OnUpdateRichEdit(mesgg + stringdata);        //将msg加入stringdata前
            }

            route[0] = 0;
            j = 0;
            while(route[j] != endpoint[1]-1)         //从0开始，路径规划是否完成
            {
                j++;
                route[j] = ma[route[j-1], 13];
            }
            for (k = 0; k < 15; k++)
            {
                mesg = "**" + route[k].ToString();
                textBox1_receive.Text += mesg;
            }
            textBox1_receive.Text += Environment.NewLine;
            
            string mesg2 = "/r/n" + "路径规划完成！" + "/r/n";
            OnUpdateRichEdit(mesg2);
        }

        uint max_cardnum = 3;      //矩阵阶数     //记得改976数值
        uint last_i = 0, card_seq = 0, line_num;           //上一次来时的卡号   card_seq 卡矩阵方位信息
        uint[] endpoint = new uint[2] { 0, 0 };   //目标点号码
        byte[] nomove_flag = new byte[] { 0xf0, 0x10, 0x20, 0x40, 0x80 };   //高四位不可行走路线
        byte[] yesmove_flag = new byte[] { 0x0f, 0x01, 0x02, 0x04, 0x08 }; //低四位已经走过的路线
        public void OnDataReceived(IAsyncResult asyn)          //数据处理函数
        {
            SocketPacket socketData = (SocketPacket)asyn.AsyncState;
            try                                               //用于检查发生的异常 
            {
                Socket workerSocket = (Socket)socketData.m_currentSocket;
                int iRx = socketData.m_currentSocket.EndReceive(asyn);         //收到的字节数目
                ip = workerSocket.RemoteEndPoint.ToString();
                if (iRx == 0)
                {
                    listSocket.Remove(ip);
                    UpdateClientListControl();
                    return;
                }
                char[] chars = new char[iRx + 1];                                         //字符串数组用于储存字符串
                System.Text.Decoder d = System.Text.Encoding.UTF8.GetDecoder();           //将字节编码成字符串
                int charLen = d.GetChars(socketData.dataBuffer, 0, iRx, chars, 0);        //解码字符并返回字符串长度
                //*******以上接收步骤，不需改变**********//

                if(socketData.dataBuffer[9] == 0x3f)                                      //判断是否为读卡信息                                
                {
                    byte[] show_bit = new byte[10];
                    byte j;
                 
                    for (byte icount = 0; icount < 10; icount++)
                        show_bit[icount] = socketData.dataBuffer[icount];

                    switch (work_mode)
                    {
                        case 0: //初始化后过程，需要搜索的卡id
                            {
                                for (byte z = 0; z < 6; z++)
                                    Aim_card[z] = show_bit[z];
                                MessageBox.Show("已读取卡片，请开启路径检测！");
                                break;
                            }
                        case 1: //路径探测过程
                            {
       //                         uint line_num;                                                      //i, 卡矩阵方位信息
                                uint dir_flag;
                                byte[] id_buffer = new byte[6];                                    //缓冲区数组
                                byte[] data_buffer = new byte[6];
                                for (j = 0; j < 6; j++)
                                    id_buffer[j] = show_bit[j];
                                
                                dir_flag = show_bit[8];                                            //上一张卡片来的方向
                                Sequence(dir_flag);
                                line_num = card_seq / (max_cardnum + 1) + 1;                          //判断本次卡片处在第几列
                                for (UInt16 thecount = 0; thecount < 6; thecount++)
                                    data_buffer[thecount] = card_data[card_seq].card_id[thecount];
                                if (BytesCompare_Step(id_buffer, data_buffer) == false)           //判断卡片没读过(比较id)
                                {
                                    for (j = 0; j < 6; j++)
                                        card_data[card_seq].card_id[j] = id_buffer[j];                //将id信息存入结构体中

                                    card_data[card_seq].card_flag[0] |= show_bit[7];                   //本张卡片可行方向记录
                                    card_data[last_i].card_flag[0] |= yesmove_flag[dir_flag];         //上张卡片可行方向
                                    card_data[card_seq].lastcard_num = last_i;                        //保持第一次来时方向信息
                                    Search_route(card_seq, line_num);
                                }
                                else                   //卡片读过
                                {
                                    card_data[card_seq].card_flag[0] |= show_bit[7];      //本张卡片可行方向记录
                                    card_data[last_i].card_flag[0] |= yesmove_flag[dir_flag];  //上张卡片可行方向
                                    if (card_seq == max_cardnum+1)                   //特殊情况
                                    {
                                        Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n"));
                                        Delay(5);
                                    }
                                    else
                                        Search_vanroute(card_seq, line_num);   //优先搜索未走路线
                                }
                                last_i = card_seq;
                                if (BytesCompare_Step(id_buffer, Aim_card) == true)                //找到卡片
                                {
                                    MessageBox.Show("找到卡片，准备信息采集！");
                                    endpoint[1] = last_i;
                                }
                                else                                                               //超声波检测是否可行
                                {
   //                                 Search_route(card_seq, line_num);
                                    Delay(2);
                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U1#\r\n"));
                                }
                                break;
                            }
                        case 2: //找到信息，路径规划过程
                            {
                                uint this_j=1, this_route=1;
                                byte[] id_buffer = new byte[6];                                    //缓冲区数组
                                byte[] data_buffer = new byte[6];
                                for (j = 0; j < 6; j++)
                                    id_buffer[j] = show_bit[j];

                                for (j = 0; j<26; j++)    //定位卡号
                                {
                                    for (UInt16 thecount = 0; thecount < 6; thecount++)
                                        data_buffer[thecount] = card_data[j+1].card_id[thecount];
                                    if(BytesCompare_Step(id_buffer, data_buffer) == true)  //没进入
                                    {
                                        this_j = j;
                                        break;
                                    }   
                                }
                                for (j=0; j<25; j++)
                                {
                                    if(this_j == route[j])   //定位卡号在已规划路径总的位置
                                    {
                                        this_route = j;
                                        break;
                                    }
                                }
                                if (BytesCompare_Step(id_buffer, Aim_card) == false)
                                {
                                    if(route[this_route+1] > route[this_route])
                                    {
                                        switch (route[this_route + 1] - route[this_route])
                                        {
                                            case 1:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n")); //北走
                                                    break;
                                                }
                                            case 3:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n")); //东走
                                                    break;
                                                }
                                        }
                                    }
                                    else
                                    {
                                        switch (route[this_route] - route[this_route+1])
                                        {
                                            case 1:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U5#\r\n")); //南走
                                                    break;
                                                }
                                            case 3:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*UA#\r\n")); //西走
                                                    break;
                                                }
                                        }
                                    }
                                    Delay(8);
                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U3#\r\n"));               //前进
                                    Delay(1);
                                }
                                else
                                {
                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*G3#\r\n"));
                                    MessageBox.Show("信息采集完成！");
                                }
                                
                                break;
                            }
                        case 3: //发生错误，回收小车过程
                            {
                                uint this_j = 1, this_route = 1;
                                byte[] id_buffer = new byte[6];                                    //缓冲区数组
                                for (j = 0; j < 6; j++)
                                    id_buffer[j] = show_bit[j];
                                for (j = 0;  j< 26; j++)
                                {
                                    if (BytesCompare_Step(id_buffer, card_data[j].card_id) == true)
                                    {
                                        this_j = j;
                                        break;
                                    }
                                }

                                for (j = 0; j < 25; j++)
                                {
                                    if (this_j == route[j])
                                    {
                                        this_route = j;
                                        break;
                                    }
                                }

                                if (route[this_route] != last_i)         /////4.11未完
                                {
                                    if (route[this_route + 1] > route[this_route])
                                    {
                                        switch (route[this_route + 1] - route[this_route])
                                        {
                                            case 1:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U4#\r\n")); //北走
                                                    break;
                                                }
                                            case 5:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U9#\r\n")); //东走
                                                    break;
                                                }
                                        }
                                    }
                                    else
                                    {
                                        switch (route[this_route] - route[this_route + 1])
                                        {
                                            case 1:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U5#\r\n")); //南走
                                                    break;
                                                }
                                            case 5:
                                                {
                                                    Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*UA#\r\n")); //西走
                                                    break;
                                                }
                                        }
                                    }

                                }
                                else
                                {
                                    MessageBox.Show("找到故障车！");
                                }
                                break;
                            }
                        case 4: //救援车（采集车）出故障，重启修复系统
                            {
                                MessageBox.Show("系统出错，请重新初始化！");
                                break;
                            }
                    }
                    /////*******信息处理，方便打印*********///
                    stringdata = BitConverter.ToString(show_bit);

                }
                else
                {
                    System.String szData = new System.String(chars);                          //字符串保存字符数组        
                    if (CurrentReceiveDataMode == DataMode.Text)          //判断hex框是否处于选中状态
                    {
                        stringdata = szData;                        //stringdata表示已收到数据
                        Error_detach(szData);
                    }
                    else
                    {
                        stringdata = ByteArrayToHexString(chars);
                        stringdata = stringdata.Substring(0, stringdata.Length - 3);
                    }

                }
                string msg = "\r\n" + "【" + ip + "】:";
                OnUpdateRichEdit(msg+ stringdata);        //将msg加入stringdata前
                WaitForData(socketData.m_currentSocket, socketData.m_clientNumber);

            }
            catch (ObjectDisposedException)
            {
                System.Diagnostics.Debugger.Log(0, "0", "\nOnDataReceived: Socket has been closed\n");
            }
            catch (SocketException se)
            {
                if (se.ErrorCode == 10054) // Error code for Connection reset by peer
                {
                    string msg = "Client " + socketData.m_clientNumber + " Disconnected" + "\r\n";
                    OnUpdateRichEdit(msg);
                    //AppendToRichEditControl(msg);
                    m_workerSocketList[socketData.m_clientNumber - 1] = null;
                    UpdateClientListControl();
                }
                else
                {
                    MessageBox.Show(se.Message);
                }
            }
        }

        private string ByteArrayToHexString(char[] data)
        {
            StringBuilder sb = new StringBuilder(data.Length * 3);
            foreach (byte b in data)
            sb.Append(Convert.ToString(b, 16).PadLeft(2, '0').PadRight(3, ' '));
            return sb.ToString().ToUpper();
        }


        private DataMode CurrentReceiveDataMode
        {
            get
            {
                return (checkReceiveMode.Checked) ? DataMode.Hex : DataMode.Text;
            }
            set
            {
                checkReceiveMode.Checked = (value == DataMode.Text);
            }
        }

        private void OnUpdateRichEdit(string msg)
        {

            textBox1_receive.Text += msg + Environment.NewLine;

        }


        void CloseSockets()             //关闭套接字
        {
            if (socket != null)
            {
                socket.Close();
            }
            Socket workerSocket = null;
            for (int i = 0; i < m_workerSocketList.Count; i++)
            {
                workerSocket = (Socket)m_workerSocketList[i];
                if (workerSocket != null)
                {
                    workerSocket.Close();
                    workerSocket = null;
                }
            }
        }

        public void Put_SendInfoToPool(Socket sendSocket)            //向sendSocker发送信息
        {
            if (PoolInfoMsg == null)
            {
                PoolInfoMsg = new ArrayList();
            }
            _tempSocket = sendSocket;
            byte[] byteArr;
            if (CurrentSendDataMode == DataMode.Hex)
            {

                byteArr = HexStringToByteArray(this.textBox3.Text + "\r\n");

            }
            else
            {

              byteArr = System.Text.Encoding.UTF8.GetBytes(this.textBox3.Text + "\r\n");
    //        byteArr = System.Text.Encoding.UTF8.GetBytes(this.textBox3.Text);

            }
            PoolInfoMsg.Add(byteArr);              //将需要发送的信息存入poolInfoMsg中          
        }

        private DataMode CurrentSendDataMode
        {
            get
            {
                return (checkReceiveMode.Checked) ? DataMode.Hex : DataMode.Text;
            }
            set
            {
                checkReceiveMode.Checked = (value == DataMode.Text);
            }
        }

        private char[] HexDigits = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f' };
        private byte[] HexStringToByteArray(string s)
        {
            StringBuilder sb = new StringBuilder(s.Length);
            foreach (char aChar in s)
            {
                if (CharInArray(aChar, HexDigits))
                    sb.Append(aChar);
            }
            s = sb.ToString();
            int bufferlength;
            if ((s.Length % 2) == 1)
                bufferlength = s.Length / 2 + 1;
            else bufferlength = s.Length / 2;
            byte[] buffer = new byte[bufferlength];
            for (int i = 0; i < bufferlength - 1; i++)
                buffer[i] = (byte)Convert.ToByte(s.Substring(2 * i, 2), 16);
            if (bufferlength > 0)
                buffer[bufferlength - 1] = (byte)Convert.ToByte(s.Substring(2 * (bufferlength - 1), (s.Length % 2 == 1 ? 1 : 2)), 16);
            return buffer;
        }

        private bool CharInArray(char aChar, char[] charArray)
        {
            return (Array.Exists<char>(charArray, delegate (char a) { return a == aChar; }));
        }

      
        private void button3_Click(object sender, EventArgs e)
        {
            textBox1_receive.Clear();
        }
     
        private void button2_Click(object sender, EventArgs e)           //选择发送的地址     
        {
            if (conected_client.Count < 1)
                MessageBox.Show("当前无可用连接");
            else
            {
                if (conected_client.Count < 2)
                {
                    //                while (Is_busy) ;
                    Put_SendInfoToPool(dicOnline[conected_client[0].ToString()]);
                }
                else
                {
                    if (work_mode == 1 || work_mode == 0)  //车一正常或者系统刚初始化
                    {
                        //                    while (Is_busy) ;
                        Put_SendInfoToPool(dicOnline[conected_client[0].ToString()]);
                    }
                    else                         //车一出错或者完成任务
                    {
                        //                   while (Is_busy) ;
                        Put_SendInfoToPool(dicOnline[conected_client[1].ToString()]);
                    }
                }
            }
            
            //if (list.SelectedItem != null)//如果已经选中了就准备发消息
            //{
            //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);               //向选中的当前字符串（client地址）发送信息
            //}
            //else
            //{
            //    MessageBox.Show("请选择接受对象");
            //}
        }

        private void Car_reset()               //初始化
        {
            textBox3.Text = "*C1RZ#";
            Put_SendInfoToPool(dicOnline[conected_client[0].ToString()]);
            Delay(8);                        //等待初始化完成
            if (conected_client.Count > 1)
            {
                Put_SendInfoToPool(dicOnline[conected_client[1].ToString()]);
                Delay(8);
            }
     //       timer1.Enabled = true;             //定时器开启计数
            
        }

        public static bool Delay(int delayTime)
        {
            DateTime now = DateTime.Now;
            int s;
            do
            {
                TimeSpan spand = DateTime.Now - now;
                s = spand.Seconds;
                Application.DoEvents();
            }
            while (s < delayTime);
            return true;
        }

       

        //窗口自动调节大小
        private ArrayList InitialCrl = new ArrayList();//用以存储窗体中所有的控件名称
        private ArrayList CrlLocationX = new ArrayList();//用以存储窗体中所有的控件原始位置
        private ArrayList CrlLocationY = new ArrayList();//用以存储窗体中所有的控件原始位置
        private ArrayList CrlSizeWidth = new ArrayList();//用以存储窗体中所有的控件原始的水平尺寸
        private ArrayList CrlSizeHeight = new ArrayList();//用以存储窗体中所有的控件原始的垂直尺寸
        private int FormSizeWidth;//用以存储窗体原始的水平尺寸
        private int FormSizeHeight;//用以存储窗体原始的垂直尺寸

        private double FormSizeChangedX;//用以存储相关父窗体/容器的水平变化量
        private double FormSizeChangedY;//用以存储相关父窗体/容器的垂直变化量 

        private int Wcounter = 0;//为防止递归遍历控件时产生混乱，故专门设定一个全局计数器
                               
        private void TcpServer_Load(object sender, EventArgs e)
        {
            GetInitialFormSize();
            GetAllCrlLocation(this);
            GetAllCrlSize(this);
        }

        public void GetAllCrlLocation(Control CrlContainer)//获得并存储窗体中各控件的初始位置
        {
            foreach (Control iCrl in CrlContainer.Controls)
            {

                if (iCrl.Controls.Count > 0)
                    GetAllCrlLocation(iCrl);
                InitialCrl.Add(iCrl);
                CrlLocationX.Add(iCrl.Location.X);
                CrlLocationY.Add(iCrl.Location.Y);


            }
        }

        public void GetAllCrlSize(Control CrlContainer)//获得并存储窗体中各控件的初始尺寸
        {
            foreach (Control iCrl in CrlContainer.Controls)
            {
                if (iCrl.Controls.Count > 0)
                    GetAllCrlSize(iCrl);
                CrlSizeWidth.Add(iCrl.Width);
                CrlSizeHeight.Add(iCrl.Height);
            }
        }

        public void GetInitialFormSize()//获得并存储窗体的初始尺寸
        {

            FormSizeWidth = this.Size.Width;
            FormSizeHeight = this.Size.Height;

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {

        }

        private void pictureBox5_Click(object sender, EventArgs e)
        {
            //  pictureBox5.Image = Image.FromFile(Directory.GetCurrentDirectory() + red.png);
        }

        private void pictureBox7_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox6_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox8_Click(object sender, EventArgs e)
        {

        }

        private void checkReceiveMode_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void label6_Click(object sender, EventArgs e)
        {

        }

        private void list_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void textBox3_KeyDown(object sender, KeyEventArgs e)
        {

        }

        private void button2_KeyDown(object sender, KeyEventArgs e)
        {
        }

        private void button2_KeyUp(object sender, KeyEventArgs e)
        {
            //textBox3.Text = "停止";
            //if (list.SelectedItem != null)//如果已经选中了就准备发消息
            //{
            //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);
            //}
            //else
            //{
            //    MessageBox.Show("请选择接受对象");
            //}
        }

        private void TcpServer_KeyDown(object sender, KeyEventArgs e)
        {
            if (Keyjudge == true)
            {
                if (e.KeyCode == Keys.Up)
                {
                    //textBox3.Text = "*U3#";
                    //if (list.SelectedItem != null)//如果已经选中了就准备发消息
                    //{
                    //    while (Is_busy) ;
                    //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);
                    //    Keyjudge = false;
                    //}
                    //else
                    //{
                    //    MessageBox.Show("请选择接受对象");
                    //}
                }

                if (e.KeyCode == Keys.Down)
                {
                    //textBox3.Text = "*U4#";
                    //if (list.SelectedItem != null)//如果已经选中了就准备发消息
                    //{
                    //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);
                    //    Keyjudge = false;
                    //}
                    //else
                    //{
                    //    MessageBox.Show("请选择接受对象");
                    //}

                }

                if (e.KeyCode == Keys.Left)
                {
                    //textBox3.Text = "*U7#";
                    //if (list.SelectedItem != null)//如果已经选中了就准备发消息
                    //{
                    //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);
                    //    Keyjudge = false;
                    //}
                    //else
                    //{
                    //    MessageBox.Show("请选择接受对象");
                    //}
                }

                if (e.KeyCode == Keys.Right)
                {
                    //textBox3.Text = "*U8#";
                    //if (list.SelectedItem != null)//如果已经选中了就准备发消息
                    //{
                    //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);
                    //    Keyjudge = false;
                    //}
                    //else
                    //{
                    //    MessageBox.Show("请选择接受对象");
                    //}
                }
            }
        }

        private void TcpServer_KeyUp(object sender, KeyEventArgs e)
        {
        if (e.KeyCode == Keys.Left || e.KeyCode == Keys.Right || e.KeyCode == Keys.Up || e.KeyCode == Keys.Down)
        {
            //textBox3.Text = "*U2#";
            //if (list.SelectedItem != null)//如果已经选中了就准备发消息
            //{
            //    Put_SendInfoToPool(dicOnline[list.SelectedItem.ToString()]);
            //    Keyjudge = true;
            //}
            //else
            //{
            //    MessageBox.Show("请选择接受对象");
            //}
        }
    }


        private void textBox1_receive_TextChanged(object sender, EventArgs e)         //textBox1的自动滚动实现
        {
            textBox1_receive.SelectionStart = textBox1_receive.Text.Length;
            textBox1_receive.ScrollToCaret();
            textBox1_receive.Focus();
        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void button4_Click(object sender, EventArgs e)
        {
            work_mode = 0;
            
           card_data = new route_data[26];
            
            for (byte i = 0; i < 26; i++)
            { 
                card_data[i].card_id = new byte[12];
                card_data[i].card_flag = new byte[1];
            }
            Car_reset();                                   
        }

        Int16 Is_ok = 0;
        //private void timer1_Tick(object sender, EventArgs e)
        //{
        //    timer1.Enabled = false;

        //    if (PoolInfoMsg == null)
        //    {
        //        PoolInfoMsg = new ArrayList();
        //    }
        //    _tempSocket = dicOnline[conected_client[0].ToString()];
        //    byte[] byteArr;
        //    byteArr = System.Text.Encoding.UTF8.GetBytes("*C2OK?#\r\n");
        //    PoolInfoMsg.Add(byteArr);
         
        //    Thread.Sleep(1000);
        //    if (stringdata.CompareTo("*CAR-OK#") != 0)
        //        Is_ok++;
        //    else Is_ok = 0;

        //    if(Is_ok > 2)                               //两次尝试连接失败，判定故障
        //    {
        //        work_mode = 3;                        //故障出现
        //        OnUpdateRichEdit("\n\rCarA error! Start CarB !");
        //        pictureBox1.Visible = false;
        //        pictureBox5.Visible = true;
        //        Is_ok = 0;
        //    }
        //    else timer1.Enabled = true;
        //}

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }

        private void button5_Click(object sender, EventArgs e)
        {
            if(work_mode == 0)            //系统刚启动，或者问题修复
            {

                work_mode = 1;
                if (card_data == null)
                {
                    card_data = new route_data[26];
                }
                for(byte i=0; i<26; i++)
                {
                    if (card_data[i].card_id == null)
                    {
                        card_data[i].card_id = new byte[12];
                    }
                    if (card_data[i].card_flag == null)
                    {
                        card_data[i].card_flag = new byte[1];
                    }
                }

                for (Int16 z = 1; z < max_cardnum * max_cardnum + 1; z++)   //限定边界
                    Marx_border(z);
                //string mesg;

                //for (byte j = 0; j < 9; j++)
                //{
                //    for (byte k = 0; k < 6; k++)
                //    {
                //        mesg = " " + card_data[j].card_id[k].ToString();
                //        textBox1_receive.Text += mesg;
                //    }
                //    mesg = " " + card_data[j].card_flag[0].ToString();
                //    textBox1_receive.Text += mesg + Environment.NewLine;
                //}

                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*G1#\r\n"));   //设置车一模式1
                Delay(2);
                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U3#\r\n"));   //前进
                Delay(2);
            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            if (work_mode != 2)          //开启路径规划标志
            {
                work_mode = 2;
                Route_plan();
                string mesg2 =  "准备开始信息采集！\r\n";
                OnUpdateRichEdit(mesg2);
                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*G2#\r\n"));
                Delay(3);
                Auto_sendmes(System.Text.Encoding.UTF8.GetBytes("*U3#\r\n"));   //北走
            }
        }

        private void TcpServer_SizeChanged(object sender, EventArgs e)
        {
            // MessageBox.Show("窗体尺寸改变");
            Wcounter = 0;
            int counter = 0;
            if (this.Size.Width < FormSizeWidth || this.Size.Height < FormSizeHeight)
            //如果窗体的大小在改变过程中小于窗体尺寸的初始值，则窗体中的各个控件自动重置为初始尺寸，且窗体自动添加滚动条
            {

                foreach (Control iniCrl in InitialCrl)
                {
                    iniCrl.Width = (int)CrlSizeWidth[counter];
                    iniCrl.Height = (int)CrlSizeHeight[counter];
                    Point point = new Point();
                    point.X = (int)CrlLocationX[counter];
                    point.Y = (int)CrlLocationY[counter];
                    iniCrl.Bounds = new Rectangle(point, iniCrl.Size);
                    counter++;
                }
                this.AutoScroll = true;
            }
            else
            //否则，重新设定窗体中所有控件的大小（窗体内所有控件的大小随窗体大小的变化而变化）
            {
                this.AutoScroll = false;
                ResetAllCrlState(this);
            }


        }
        public void ResetAllCrlState(Control CrlContainer)//重新设定窗体中各控件的状态（在与原状态的对比中计算而来）
        {


            FormSizeChangedX = (double)this.Size.Width / (double)FormSizeWidth;
            FormSizeChangedY = (double)this.Size.Height / (double)FormSizeHeight;

            foreach (Control kCrl in CrlContainer.Controls)
            {

                if (kCrl.Controls.Count > 0)
                {
                    ResetAllCrlState(kCrl);

                }

                Point point = new Point();
                point.X = (int)((int)CrlLocationX[Wcounter] * FormSizeChangedX);
                point.Y = (int)((int)CrlLocationY[Wcounter] * FormSizeChangedY);
                kCrl.Width = (int)((int)CrlSizeWidth[Wcounter] * FormSizeChangedX);
                kCrl.Height = (int)((int)CrlSizeHeight[Wcounter] * FormSizeChangedY);
                kCrl.Bounds = new Rectangle(point, kCrl.Size);
                Wcounter++;
            }
        }


    }



}


