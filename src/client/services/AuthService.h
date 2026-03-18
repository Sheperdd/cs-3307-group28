#pragma once

#include <QObject>
#include <QString>

#include "../api/TorqueNetworkManager.h"
#include "../../engine/DTO.h"

class AuthService : public QObject
{
  Q_OBJECT

public:
  // Pass the network manager via Dependency Injection
  explicit AuthService(TorqueNetworkManager *networkManager, QObject *parent = nullptr);

  /// @brief Registers a new user account. The server will determine the role based on the isMechanic flag.
  /// @param fullName full name of the user
  /// @param email email address (must be unique)
  /// @param phone phone number
  /// @param password plaintext password (the server will hash it before storing)
  /// @param isMechanic if true, the server will create a mechanic profile for this user; otherwise, it's a customer account
  void registerAccount(const std::string &fullName, const std::string &email,
                       const std::string &phone, const std::string &password,
                       bool isMechanic = false);

  // Called by the UI layer (e.g., LoginPage)
  void login(const std::string &email, const std::string &password);

signals:
  void registerSuccessful(const AuthResult &result);
  void registerFailed(const QString &errorMessage);

  // Emitted back to the UI when the network request finishes
  void loginSuccessful(const AuthResult &result); // pass the parsed DTO back to the UI
  void loginFailed(const QString &errorMessage);

private slots:
  void onRegisterFinished(); // Intercepts the specific reply for the registration request
  void onLoginFinished();    // Intercepts the specific reply for the login request

private:
  TorqueNetworkManager *network;
};
