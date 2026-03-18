// --------------------------------------------------------------------------
// This file is responsible for handling all user authentication.
// It acts as a middleman between the User Interface (the screens) and
// the server (the backend). It takes the user's typed information, packages
// it up, sends it over the internet, and tells the UI if it worked or failed.
// --------------------------------------------------------------------------

#include "AuthService.h"
#include <QNetworkReply> // Let's us receive responses from our web requests
#include <nlohmann/json.hpp>
#include "../../engine/DTOSerialization.h" // DTOs (Data Transfer Objects) define the shapes of our data

// This creates a shortcut so we can just type 'json' instead of 'nlohmann::json' every time
using json = nlohmann::json;

// --------------------------------------------------------------------------
// Constructor: This runs exactly once when an AuthService is created.
// --------------------------------------------------------------------------
AuthService::AuthService(TorqueNetworkManager *networkManager, QObject *parent)
    // We pass 'parent' to QObject so Qt automatically cleans up memory when we close the app.
    // We also save the 'networkManager' so this service can make internet requests later.
    : QObject(parent), network(networkManager)
{
}

// --------------------------------------------------------------------------
// registerAccount: Takes the information the user typed into the sign-up screen
// and sends it to the server to create a new account.
// --------------------------------------------------------------------------
void AuthService::registerAccount(const std::string &fullName, const std::string &email,
                                  const std::string &phone, const std::string &password,
                                  bool isMechanic)
{
  // 1. Build a JSON object. JSON is just a text format that web servers easily understand.
  json j;
  j["fullName"] = fullName;
  j["email"] = email;
  j["phone"] = phone;
  j["password"] = password;
  // If true, label them a mechanic, otherwise label them a customer
  j["isMechanic"] = isMechanic ? "mechanic" : "customer";

  // 2. Convert that JSON object into plain text, and then into "Bytes" (raw data)
  // because that is what the network manager needs to send it over the internet.
  std::string jsonString = j.dump();
  QByteArray body = QByteArray::fromStdString(jsonString);

  // 3. Send the request! We tell our network manager to POST (send new data) to the "/auth/register" URL.
  // It gives us back a "reply" object which we can monitor to see when the server eventually responds.
  QNetworkReply *reply = network->post("/auth/register", body);

  // 4. Set an alarm (Connection). We tell Qt: "When this 'reply' announces it is completely finished downloading,
  // please run my 'onRegisterFinished' function."
  connect(reply, &QNetworkReply::finished, this, &AuthService::onRegisterFinished);
}

// --------------------------------------------------------------------------
// onRegisterFinished: This function wakes up automatically the moment the
// server answers our registration request.
// --------------------------------------------------------------------------
void AuthService::onRegisterFinished()
{
  // 1. Figure out which network request just finished. (sender() gets the object that woke us up)
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

  // If it's empty (something went terribly wrong inside Qt), just stop here.
  if (!reply)
    return;

  // Tell Qt to delete this reply object from memory later when it's safe to do so.
  reply->deleteLater();

  // 2. Check if the web request was successful (No network failures like "No Internet Connection")
  if (reply->error() == QNetworkReply::NoError)
  {
    try
    {
      // Read everything the server sent back to us
      std::string responseData = reply->readAll().toStdString();

      // Convert the server's text response back into a JSON object
      json j = json::parse(responseData);

      // Convert the JSON automatically into our C++ 'AuthResult' structure.
      AuthResult result = j.get<AuthResult>();

      // A 'token' is like a digital VIP wristband. The server gave us one because we registered successfully.
      // We give it to the network manager so it can prove who we are on all future requests.
      network->setAuthToken(QString::fromStdString(result.token));

      // Shout out to the UI (emit a signal): "Registration worked! Here is the user data!"
      // The UI screen will hear this and probably switch to the Home screen.
      emit registerSuccessful(result);
    }
    catch (const json::exception &e)
    {
      // If we got here, the server responded, but what it sent wasn't valid JSON.
      // Shout out to the UI: "Something broke trying to read the response!"
      emit registerFailed("Data mismatch error: Could not read server response.");
    }
  }
  else // 3. The request failed (maybe wrong inputs, server down, etc.)
  {
    // Ask the reply what the specific HTTP Error Code was (e.g., 404, 500)
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // 400 means "Bad Request" - usually meaning the user typed an email that already exists.
    if (statusCode == 400)
    {
      emit registerFailed("Invalid data provided. Email may already be in use.");
    }
    else
    {
      emit registerFailed("Registration failed. Please try again.");
    }
  }
}

// --------------------------------------------------------------------------
// login: Takes an email and password from the login screen, packs it up,
// and asks the server if it matches a valid user.
// --------------------------------------------------------------------------
void AuthService::login(const std::string &email, const std::string &password)
{
  // 1. Build a JSON object containing just the login credentials
  json j;
  j["email"] = email;
  j["password"] = password;

  // 2. Convert to raw bytes so it can be sent over the internet
  std::string jsonString = j.dump();
  QByteArray body = QByteArray::fromStdString(jsonString);

  // 3. Send a POST request to the "/auth/login" URL.
  QNetworkReply *reply = network->post("/auth/login", body);

  // 4. Set our alarm. When the server replies, run 'onLoginFinished'.
  connect(reply, &QNetworkReply::finished, this, &AuthService::onLoginFinished);
}

// --------------------------------------------------------------------------
// onLoginFinished: Wakes up when the server replies to our login attempt.
// --------------------------------------------------------------------------
void AuthService::onLoginFinished()
{
  // 1. Grab the reply object that triggered this function
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  if (!reply)
    return;

  // Mark it for deletion so we don't leak memory
  reply->deleteLater();

  // 2. Did the request succeed?
  if (reply->error() == QNetworkReply::NoError)
  {
    try
    {
      // Read the server's response
      std::string responseData = reply->readAll().toStdString();
      json j = json::parse(responseData);

      AuthResult result;

      // If the server gave us a "token" (our digital VIP wristband), save it so
      // we stay logged in for future requests.
      if (j.contains("token"))
      {
        network->setAuthToken(QString::fromStdString(j["token"].get<std::string>()));
      }

      // Tell the UI that the login was successful!
      emit loginSuccessful(result);
    }
    catch (const json::exception &e)
    {
      // The server sent weird data we couldn't understand
      emit loginFailed("Data mismatch error: Could not read server response.");
    }
  }
  else // 3. The request failed. Usually means wrong password!
  {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // 401 means "Unauthorized". The server said our email/password combo is wrong.
    if (statusCode == 401)
    {
      emit loginFailed("Invalid email or password.");
    }
    else
    {
      emit loginFailed("Login failed. Please try again.");
    }
  }
}
