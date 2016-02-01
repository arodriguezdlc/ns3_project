/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef G711_GENERATOR_HELPER_H
#define G711_GENERATOR_HELPER_H

#include <stdint.h>
#include <string>
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

namespace ns3 {

/**
 * \ingroup bulksend
 * \brief A helper to make it easier to instantiate an ns3::G711Generator
 * on a set of nodes.
 */
class G711GeneratorHelper
{
public:
  /**
   * Create an G711GeneratorHelper to make it easier to work with G711Generator
   *
   * \param protocol the name of the protocol to use to send traffic
   *        by the applications. This string identifies the socket
   *        factory type used to create sockets for the applications.
   *        A typical value would be ns3::UdpSocketFactory.
   * \param address the address of the remote node to send traffic
   *        to.
   */
  G711GeneratorHelper (std::string protocol, Address address);

  /**
   * Helper function used to set the underlying application attributes, 
   * _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Install an G711Generator on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an G711Generator
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * Install an G711Generator on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an G711Generator will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Install an G711Generator on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param nodeName The node on which an G711Generator will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * Install an G711Generator on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an G711Generator will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* G711_GENERATOR_HELPER_H */
