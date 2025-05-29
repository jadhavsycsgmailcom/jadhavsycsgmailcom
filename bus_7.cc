/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */  

// Add required header files  
#include "ns3/core-module.h"  
#include "ns3/network-module.h"  
#include "ns3/point-to-point-module.h"  
#include "ns3/internet-module.h"  
#include "ns3/applications-module.h" 
#include "ns3/netanim-module.h"  
#include "ns3/csma-module.h"  
#include "ns3/ipv4-global-routing-helper.h"  

// Add namespace using 
using namespace ns3;  

// Define log component to store log messages  
NS_LOG_COMPONENT_DEFINE("BusTopology");  

// Main function  
int main(int argc, char *argv[]) 
{     
  uint32_t extra_nodes = 3; // Declare no. of nodes in bus topology  

  // Read and process command-line arguments     
  CommandLine cmd(__FILE__);     
  cmd.Parse(argc, argv);  

  // Set time resolution  
  Time::SetResolution(Time::NS);  

  // Enable logging for server and client applications  
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);  
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);  

  // Create point-to-point topology     
  NodeContainer p2pNodes;     
  p2pNodes.Create(2); // Create 2 nodes in point-to-point topology  

  // Configure point-to-point net devices and channel     
  PointToPointHelper pointToPoint;     
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));     
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));  

  // Install configured point-to-point net devices on nodes     
  NetDeviceContainer p2pDevices;     
  p2pDevices = pointToPoint.Install(p2pNodes);  

  // Create bus topology     
  NodeContainer busNodes;     
  busNodes.Add(p2pNodes.Get(1)); // Add node n1 to bus topology     
  busNodes.Create(extra_nodes);  // Create additional nodes (n2, n3, n4)  

  // Configure CSMA devices and channels  
  CsmaHelper csma;  
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));     
  csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));  

  // Install net devices on bus nodes and connect them with the channel     
  NetDeviceContainer busDevices;     
  busDevices = csma.Install(busNodes);  

  // Install protocol stack on all nodes     
  InternetStackHelper stack;     
  stack.Install(p2pNodes.Get(0)); // Install stack on n0     
  stack.Install(busNodes); // Install stack on n1, n2, n3, n4  

  // Configure IP addresses  
  Ipv4AddressHelper address;  

  // Assign IP addresses to P2P nodes     
  address.SetBase("10.0.0.0", "255.255.255.0");  
  Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);  

  // Assign IP addresses to bus nodes     
  address.SetBase("20.0.0.0", "255.255.255.0");  
  Ipv4InterfaceContainer busInterfaces = address.Assign(busDevices);  

  // Configure server application on node n4 (last node in bus topology)  
  UdpEchoServerHelper echoServer(9);  
  ApplicationContainer serverApp = echoServer.Install(busNodes.Get(extra_nodes));  

  // Configure start and stop time for the server     
  serverApp.Start(Seconds(1.0));     
  serverApp.Stop(Seconds(10.0));  

  // Configure client application on node n0  
  UdpEchoClientHelper echoClient(busInterfaces.GetAddress(extra_nodes), 9);     
  echoClient.SetAttribute("MaxPackets", UintegerValue(2));     
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));     
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));  
  ApplicationContainer clientApp = echoClient.Install(p2pNodes.Get(0));  

  // Configure start and stop time for the client     
  clientApp.Start(Seconds(2.0));     
  clientApp.Stop(Seconds(10.0));  

  // Enable routing between networks 10.0.0.0 and 20.0.0.0  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables(); 

  // Capture packets on all nodes in P2P topology     
  pointToPoint.EnablePcapAll("p2p_Packet");  

  // Capture packets on node n2 in bus topology  
  csma.EnablePcap("bus_packet", busDevices.Get(1), true);  

  // Enable NetAnim for visualization  
  AnimationInterface anim("bus-topology.xml");  

  // Set positions of nodes in bus topology     
  anim.SetConstantPosition(p2pNodes.Get(0), 10.0, 15.0);     
  anim.SetConstantPosition(busNodes.Get(0), 30.0, 15.0);     
  anim.SetConstantPosition(busNodes.Get(1), 40.0, 15.0);     
  anim.SetConstantPosition(busNodes.Get(2), 50.0, 15.0);     
  anim.SetConstantPosition(busNodes.Get(3), 60.0, 15.0);  

  // Run simulation and destroy resources  
  Simulator::Run();  
  Simulator::Destroy();  

  return 0; 
}

