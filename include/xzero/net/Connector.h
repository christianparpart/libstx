#pragma once

#include <xzero/Api.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <list>

namespace xzero {

class Server;
class Executor;
class EndPoint;
class ConnectionFactory;
class ConnectionListener;

/**
 * Base API for accepting new clients.
 */
class XZERO_API Connector {
 public:
  /**
   * Initializes this connector.
   */
  Connector(const std::string& name, Executor* executor);

  virtual ~Connector();

  /**
   * Assigns a @p server to this connector.
   */
  void setServer(Server* server);

  /**
   * Retrieves the corresponding Server object.
   */
  Server* server() const;

  /**
   * Retrieves the describing name for this connector.
   */
  const std::string& name() const;

  /**
   * Sets a descriptive connector name.
   */
  void setName(const std::string& name);

  /**
   * Starts given connector.
   *
   * @throw std::runtime_error on runtime errors
   */
  virtual void start() = 0;

  /**
   * Tests whether this connector has been started.
   */
  virtual bool isStarted() const noexcept = 0;

  /**
   * Stops given connector.
   */
  virtual void stop() = 0;

  /**
   * Retrieves list of currently connected endpoints.
   */
  virtual std::list<EndPoint*> connectedEndPoints() = 0;

  /**
   * Registeres a new connection factory.
   */
  std::shared_ptr<ConnectionFactory> addConnectionFactory(
      std::shared_ptr<ConnectionFactory> factory);

  /**
   * Registeres a new connection factory.
   */
  template <typename T, typename... Args>
  std::shared_ptr<T> addConnectionFactory(Args... args);

  /**
   * Retrieves associated connection factory by @p protocolName.
   *
   * @param protocolName protocol name for the connection factory to retrieve.
   */
  std::shared_ptr<ConnectionFactory> connectionFactory(
      const std::string& protocolName) const;

  /** Retrieves all registered connection factories. */
  std::list<std::shared_ptr<ConnectionFactory>> connectionFactories() const;

  /**
   * Sets the default connection factory.
   */
  void setDefaultConnectionFactory(std::shared_ptr<ConnectionFactory> factory);

  /**
   * Retrieves the default connection factory.
   */
  std::shared_ptr<ConnectionFactory> defaultConnectionFactory() const;

  /**
   * Retrieves the default task executor service.
   */
  Executor* executor() const { return executor_; }

  /**
   * Adds an Connection listener that will be
   * automatically associated to newly created connections.
   */
  void addListener(ConnectionListener* listener);

  /** Retrieves list of connection listeners. */
  const std::list<ConnectionListener*>& listeners() const { return listeners_; }

 private:
  std::string name_;
  Server* server_;
  Executor* executor_;

  std::unordered_map<std::string, std::shared_ptr<ConnectionFactory>>
      connectionFactories_;

  std::shared_ptr<ConnectionFactory> defaultConnectionFactory_;

  std::list<ConnectionListener*> listeners_;
};

template <typename T, typename... Args>
inline std::shared_ptr<T> Connector::addConnectionFactory(Args... args) {
  return std::static_pointer_cast<T>(
      addConnectionFactory(std::shared_ptr<T>(new T(args...))));
}

}  // namespace xzero
