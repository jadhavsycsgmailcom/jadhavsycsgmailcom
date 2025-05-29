/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */  

#include "ns3/core-module.h"            
#include "ns3/network-module.h"        
#include "ns3/internet-module.h"    
#include "ns3/point-to-point-module.h"   
#include "ns3/applications-module.h"    
#include "ns3/netanim-module.h"         
#include "ns3/point-to-point-layout-module.h"  

using namespace ns3;  

NS_LOG_COMPONENT_DEFINE("Star");   

int main(int argc, char *argv[])  
{  
  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(137));  
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("14kb/s"));  

  // Define spoke nodes  
  uint32_t nSpokes = 8; // Number of peripheral nodes 

  // Command line arguments  
  CommandLine cmd(__FILE__);     
  cmd.Parse(argc, argv);  

  // Configure point-to-point parameters  
  PointToPointHelper pointToPoint;     
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));     
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));  

  // Create star topology   
  PointToPointStarHelper star(nSpokes, pointToPoint);  

  NS_LOG_INFO("STAR TOPOLOGY CREATED!");  

  // Install internet stack     
  InternetStackHelper stack;     
  star.InstallStack(stack);  

  NS_LOG_INFO("INSTALLED PROTOCOL STACK ON ALL NODES IN TOPOLOGY!");  

  // Assign IP addresses  
  star.AssignIpv4Addresses(Ipv4AddressHelper("10.0.0.0", "255.0.0.0"));  
  NS_LOG_INFO("IP V4 ADDRESSES ARE ASSIGNED TO SPOKE NODES AND HUB");  

  // Create applications     
  uint16_t port = 50000;   

  // Configure packet sink at hub  
  PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory",   
                                    InetSocketAddress(Ipv4Address::GetAny(), port));  
  ApplicationContainer hubApp = packetSinkHelper.Install(star.GetHub());     
  hubApp.Start(Seconds(1.0));     
  hubApp.Stop(Seconds(10.0));  

  // Configure OnOff applications for spoke nodes  
  OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());  
  onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));     
  onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));  

  ApplicationContainer spokeApps;  
  for (uint32_t i = 0; i < star.SpokeCount(); i++)  
  { 
    AddressValue remoteAddress(InetSocketAddress(star.GetHubIpv4Address(i), port));  
    onOffHelper.SetAttribute("Remote", remoteAddress);         
    spokeApps.Add(onOffHelper.Install(star.GetSpokeNode(i)));  
  }   

  // Configure start and stop time of ON/OFF Application     
  spokeApps.Start(Seconds(1.0));     
  spokeApps.Stop(Seconds(10.0));  

  // Enable pcap tracing     
  pointToPoint.EnablePcapAll("star");  

  // Enable animation  
  AnimationInterface anim("star_topology.xml");  

  // Animate star topology     
  star.BoundingBox(1, 1, 100, 100);  

  // Run simulation  
  NS_LOG_INFO("RUNNING THE SIMULATION");  
  Simulator::Stop(Seconds(10.0));  
  Simulator::Run();  
  Simulator::Destroy();  
  NS_LOG_INFO("SIMULATION COMPLETED");  

  return 0;  
}

