/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <string>
#include <cstring>
#include <fstream>

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SixthScriptExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
//
// We want to look at changes in the ns-3 TCP congestion window.  We need
// to crank up a flow and hook the CongestionWindow attribute on the socket
// of the sender.  Normally one would use an on-off application to generate a
// flow, but this has a couple of problems.  First, the socket of the on-off
// application is not created until Application Start time, so we wouldn't be
// able to hook the socket (now) at configuration time.  Second, even if we
// could arrange a call after start time, the socket is not public so we
// couldn't get at it.
//
// So, we can cook up a simple version of the on-off application that does what
// we want.  On the plus side we don't need all of the complexity of the on-off
// application.  On the minus side, we don't have a helper, so we have to get
// a little more involved in the details, but this is trivial.
//
// So first, we create a socket and do the trace connect on it; then we pass
// this socket into the constructor of our simple application which we then
// install in the source node.
// ===========================================================================
//
int fa = 1;


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

uint8_t img_fill[561][1446];
int cnt = 0;
int packet_cnt[10000];
int packet_cnt_cnt[10000];
void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (img_fill[cnt], m_packetSize);
  // std::cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<std::endl;
  // std::cout<<"Send time: "<< Simulator::Now ().GetSeconds ()<<", Packet cnt: "<<++cnt<<" ,"<<" Send Packet UID: "<< packet->GetUid()<<std::endl;
  m_socket->Send (packet);
  packet_cnt[packet->GetUid()]=++cnt;
  // std::cout<<cnt<<std::endl;
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}


void
MyApp::ScheduleTx (void)
{
  // if (fa==1){
	//   std::cout<<m_packetSize* 8/static_cast<double> (m_dataRate.GetBitRate ())<<std::endl;
  // }
  fa=0;
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

int size =0;
int out_arr[562][1446];
static void //drop
RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
  uint8_t *buffer = new uint8_t[p->GetSize ()];
  size = p->CopyData(buffer, p->GetSize ());
  // p->GetUid()
  std::cout<<std::endl<<p->GetUid()<<", "<<packet_cnt[p->GetUid()];
  std::cout<<"******************************"<<std::endl;
  if (size == 1500)
  {
    for (int i = 54; i < size; i++)
    {
      if (packet_cnt[p->GetUid()] == 561)
      {
        if (packet_cnt_cnt[p->GetUid()] == 240)
        {
          break;
        }
        out_arr[packet_cnt[p->GetUid()]][packet_cnt_cnt[p->GetUid()]] = -1;
        packet_cnt_cnt[p->GetUid()] += 1;
      }
      // std::cout<<buffer[i]+p->GetSize()-size<<",";
      else
      {
        out_arr[packet_cnt[p->GetUid()]][packet_cnt_cnt[p->GetUid()]] = -1;
        packet_cnt_cnt[p->GetUid()] += 1;
      }
    }
  }
  // std::cout<<(packet_cnt_cnt[p->GetUid()])<<" , "<<out_arr[packet_cnt[p->GetUid()]][packet_cnt_cnt[p->GetUid()]];
  // std::cout<<std::dec<<"Drop Packet Size: "<<size<<" Packet Uid: "<<p->GetUid()<<std::endl;
  // std::cout<<packet_cnt[p->GetUid()]<<std::endl;
  // NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
  file->Write (Simulator::Now (), p);
}

static void
RxEnd(Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
  uint8_t *buffer = new uint8_t[p->GetSize()];
  size = p->CopyData(buffer, p->GetSize());
  // std::cout<<"======================================"<<std::endl;
  if (size == 1500)
  {
    for (int i = 54; i < size; i++)
    {
      if (packet_cnt[p->GetUid()] == 561)
      {
        if (packet_cnt_cnt[p->GetUid()] == 240)
        {
          break;
        }
        out_arr[packet_cnt[p->GetUid()]][packet_cnt_cnt[p->GetUid()]] = buffer[i] + p->GetSize() - size;
        packet_cnt_cnt[p->GetUid()] += 1;
      }
      // std::cout<<buffer[i]+p->GetSize()-size<<",";
      else
      {
        out_arr[packet_cnt[p->GetUid()]][packet_cnt_cnt[p->GetUid()]] = buffer[i] + p->GetSize() - size;
        packet_cnt_cnt[p->GetUid()] += 1;
      }
    }
  }
  // std::cout<<std::endl;
  // std::cout<<std::dec<<"Receive at " << Simulator::Now ().GetSeconds ()<<", Packet size: "<<size<<" Packet Uid: "<<p->GetUid()<<std::endl;

  file->Write(Simulator::Now(), p);
}

int
main (int argc, char *argv[])
{
  int width = 450;
  int height = 600;
  int channel = 3;
  int data_len = width * height * channel;
  int packet_sss = 1446;
  std::ifstream read;
  read.open("x_out.txt");
  std::string sa;
  int read_cnt = 0;
  while(getline(read,sa)) 
  {
    int i = read_cnt/ packet_sss;
    int j = read_cnt % packet_sss;
    int abc= atoi(sa.c_str());
    img_fill[i][j]=abc;
    read_cnt++;
  }
  read.close();
  

  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetDeviceAttribute ("Mtu", StringValue ("1500"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET));
  em->SetAttribute ("ErrorRate", DoubleValue (0.01));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  

  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  
  ns3TcpSocket->SetAttribute("SegmentSize",  StringValue("3000"));
  

  int send_packet_num = data_len / packet_sss + 1; 
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, packet_sss, send_packet_num, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));


  PcapHelper pcapHelper;
  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile ("sixth.pcap", std::ios::out, PcapHelper::DLT_PPP);
  devices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&RxDrop, file));

  Ptr<PcapFileWrapper> file1 = pcapHelper.CreateFile ("sixth_R.pcap", std::ios::out, PcapHelper::DLT_PPP);
  devices.Get (1)->TraceConnectWithoutContext ("PhyRxEnd", MakeBoundCallback (&RxEnd, file));

  pointToPoint.EnablePcapAll ("abc-p2p");

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();

  // width = 500;
  // height = 375;
  std::ofstream writeF("result_1.txt");
  int cnt_write=0;
  bool flag11=true;
  for(int i=1;i<=send_packet_num;i++)
  {
    for (int j=0; j<packet_sss;j++)
    {
      
      std::string outline = std::to_string(out_arr[i][j]);      
      if(cnt==data_len) // h * w * c 
      {
        writeF<<outline;
        flag11=false;
        break;
      }
      else
      {
        writeF<< outline<<std::endl;
        cnt_write++;
      }
    }
    if(flag11==false)
    {
      break;
    }
  }
  writeF.close();
  
  return 0;
}

