/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef HTTP_GENERATOR_SERVER_HELPER_H
#define HTTP_GENERATOR_SERVER_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

namespace ns3 {

/**
 * \ingroup packetsink
 * \brief A helper to make it easier to instantiate an ns3::HttpGeneratorServer
 * on a set of nodes.
 */
class HttpGeneratorServerHelper
{
public:
  /**
   * Create a HttpGeneratorServerHelper to make it easier to work with HttpGeneratorServers
   *
   * \param protocol the name of the protocol to use to receive traffic
   *        This string identifies the socket factory type used to create
   *        sockets for the applications.  A typical value would be 
   *        ns3::TcpSocketFactory.
   * \param address the address of the sink,
   *
   */
  HttpGeneratorServerHelper (std::string protocol, Address address);

  /**
   * Helper function used to set the underlying application attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Install an  HttpGeneratorServer on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which a HttpGeneratorServer 
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * Install an HttpGeneratorServer on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param node The node on which a HttpGeneratorServer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Install an HttpGeneratorServer on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param nodeName The name of the node on which a HttpGeneratorServer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * Install an HttpGeneratorServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an HttpGeneratorServer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* HTTP_GENERATOR_SERVER_HELPER_H */
