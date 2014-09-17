#pragma once

#include <xzero/Api.h>
#include <xzero/Buffer.h>
#include <xzero/net/IPAddress.h>

#include <initializer_list>
#include <list>
#include <memory>

namespace xzero {

class Connector;

/**
 * General purpose Server.
 */
class XZERO_API Server {
 public:
  /**
   * Minimally initializes a Server without any listener.
   *
   * @see addConnector(std::unique_ptr<Connector>&& connector)
   */
  Server();

  /**
   * Initializes the server with one server connector, listening on given
   * @p port for any IP.
   *
   * @param port the port number to initially listen on.
   */
  explicit Server(int port);

  /**
   * Initializes the server with one server connector, listening on given
   * @p address and given @p port.
   *
   * @param address IP address to bind to
   * @param port TCP port number to listen on
   */
  Server(const IPAddress& address, int port);

  /**
   * Destructs this server.
   */
  ~Server();

  /**
   * Starts all connectors.
   */
  void start();

  /**
   * Stops all connectors.
   */
  void stop();

  /**
   * Creates and adds a new connector of type @c T to this server.
   *
   * @param args list of arguments being passed to the connectors constructor.
   */
  template <typename T, typename... Args>
  T* addConnector(Args... args);

  /**
   * Adds given connector to this server.
   *
   * @return raw pointer to the connector.
   */
  template <typename T>
  T* addConnector(std::unique_ptr<T>&& connector) {
    implAddConnector(connector.get());
    return connector.release();
  }

  /**
   * Removes given @p connector from this server.
   */
  void removeConnector(Connector* connector);

  /**
   * Retrieves list of all registered connectors.
   */
  std::list<Connector*> getConnectors() const;

  /**
   * Fills given buffer with the current date.
   *
   * Use this to generate Date response header field values.
   */
  void getDate(char* buf, size_t size);

 private:
  void implAddConnector(Connector* connector);

 private:
  std::list<Connector*> connectors_;
  Buffer date_;
};

template <typename T, typename... Args>
T* Server::addConnector(Args... args) {
  return static_cast<T*>(addConnector(std::unique_ptr<T>(new T(std::forward<Args>(args)...))));
}

}  // namespace xzero
