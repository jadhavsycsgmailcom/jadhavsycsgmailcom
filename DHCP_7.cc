/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */  
#include "ns3/core-module.h"           
#include "ns3/network-module.h"     
#include "ns3/internet-apps-module.h"       
#include "ns3/internet-module.h"    
#include "ns3/point-to-point-module.h"   
#include "ns3/applications-module.h"    
#include "ns3/ipv4-global-routing-helper.h"       
#include "ns3/csma-module.h"      

using namespace ns3;  

NS_LOG_COMPONENT_DEFINE("DHCP_Example");   

int main(int argc, char *argv[])  
{  
  CommandLine cmd(__FILE__);     
  cmd.Parse(argc, argv);  

  Time::SetResolution(Time::NS);  

  // Enable logging for applications  
  LogComponentEnable("DhcpServer", LOG_LEVEL_ALL);  
  LogComponentEnable("DhcpClient", LOG_LEVEL_ALL);  
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);  
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);  

  // Create nodes in bus topology     
  NS_LOG_INFO("Create nodes");  
  NodeContainer nodes; // DHCP client nodes n0, n1, n2  
  NodeContainer router; // Router nodes r0, r1  
  nodes.Create(3);     
  router.Create(2);  

  // Combine routers and nodes into a single object  
  NodeContainer bus_nodes(nodes, router);  

  // Configure the CSMA network     
  CsmaHelper csma;     
  csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));     
  csma.SetChannelAttribute("Delay", StringValue("2ms"));     
  csma.SetDeviceAttribute("Mtu", UintegerValue(1500));  

  // Install CSMA devices on bus nodes and connect them with a channel  
  NetDeviceContainer busDevices = csma.Install(bus_nodes);  

  // Create point-to-point topology  
  NodeContainer p2pNodes; // Router r1 and Node A  
  p2pNodes.Add(bus_nodes.Get(4)); // Add router r1  
  p2pNodes.Create(1); // Create new Node A  

  // Configure point-to-point network     
  PointToPointHelper pointToPoint;     
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));     
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));  

  // Install configured P2P network on p2p nodes  
  NetDeviceContainer p2pDevices = pointToPoint.Install(p2pNodes);  

  // Install protocol stack on all nodes     
  InternetStackHelper stack;     
  stack.Install(nodes);       // Install on n0, n1, n2     
  stack.Install(router);      // Install on r0, r1  
  stack.Install(p2pNodes.Get(1));  // Install on Node A       

  // Configure IP address for P2P devices     
  Ipv4AddressHelper address;  
  address.SetBase("20.0.0.0", "255.0.0.0");  

  // Assign IP addresses to P2P devices  
  Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);  

  // Assign a fixed IP address to the default router r1  
  DhcpHelper dhcpHelper;  
  Ipv4InterfaceContainer fixedInterface = dhcpHelper.InstallFixedAddress(         
    busDevices.Get(4), Ipv4Address("10.0.0.17"), Ipv4Mask("/8")  
  );  

  // Enable forwarding of packets from r1     
  fixedInterface.Get(0).first->SetAttribute("IpForward", BooleanValue(true));  

  // Enable routing between networks  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();  

  // Configure and install DHCP server application on router r0     
  ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer(         
    busDevices.Get(3), 
    Ipv4Address("10.0.0.12"), 
    Ipv4Address("10.0.0.0"),   
    Ipv4Mask("/8"), 
    Ipv4Address("10.0.0.1"), 
    Ipv4Address("10.0.0.15"),   
    Ipv4Address("10.0.0.17")  
  );  

  // Configure start & stop time of server     
  dhcpServerApp.Start(Seconds(0.0));     
  dhcpServerApp.Stop(Seconds(20.0));     

  // Combine net devices of nodes n0, n1, n2 into a single object     
  NetDeviceContainer dhcpClientNetDevs;     
  dhcpClientNetDevs.Add(busDevices.Get(0));     
  dhcpClientNetDevs.Add(busDevices.Get(1));     
  dhcpClientNetDevs.Add(busDevices.Get(2));  

  // Install DHCP clients on n0, n1, n2  
  ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient(dhcpClientNetDevs);  

  // Configure start & stop time of DHCP client     
  dhcpClients.Start(Seconds(1.0));     
  dhcpClients.Stop(Seconds(20.0));  

  // Configure and install UdpEchoServerApplication on Node A  
  UdpEchoServerHelper echoServer(9);  // Port 9  
  ApplicationContainer serverApp = echoServer.Install(p2pNodes.Get(1));     
  serverApp.Start(Seconds(2.0));     
  serverApp.Stop(Seconds(20.0));  

  // Configure and install UdpEchoClientApplication on Node n1  
  UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);  // Destination: Node A, Port 9  
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));     
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));     
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));  

  ApplicationContainer clientApp = echoClient.Install(nodes.Get(1));  // Install on n1     
  clientApp.Start(Seconds(3.0));     
  clientApp.Stop(Seconds(20.0));  

  // Run simulation  
  NS_LOG_INFO("Run Simulation");  
  Simulator::Run();  

  // Destroy resources  
  NS_LOG_INFO("Done");  
  Simulator::Destroy();  

  return 0;  
}

