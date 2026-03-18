#include "TorqueNetworkManager.h"

TorqueNetworkManager::TorqueNetworkManager(QObject *parent)
    : QObject(parent), baseUrl("http://localhost:6967")
{

  // Initialize the Qt network engine
  manager = new QNetworkAccessManager(this);

  // Centralized Error Handling:
  // Whenever the manager finishes ANY request, route it through our custom slot first
  connect(manager, &QNetworkAccessManager::finished,
          this, &TorqueNetworkManager::onReplyFinished);
}

void TorqueNetworkManager::setAuthToken(const QString &token)
{
  authToken = token;
}

void TorqueNetworkManager::clearAuthToken()
{
  authToken.clear();
}

// Builds the base request so we don't have to repeat this code in get/post/patch
QNetworkRequest TorqueNetworkManager::createRequest(const QString &endpoint)
{
  QNetworkRequest request(QUrl(baseUrl + endpoint));

  // Tell the server we are sending and expecting JSON
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  // If the user is logged in, attach their JWT to the Authorization header
  if (!authToken.isEmpty())
  {
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());
  }

  return request;
}

// Example of how the verbs are implemented
QNetworkReply *TorqueNetworkManager::get(const QString &endpoint)
{
  QNetworkRequest request = createRequest(endpoint);
  return manager->get(request); // Fires the request asynchronously
}

// This runs automatically every time a network request finishes
void TorqueNetworkManager::onReplyFinished(QNetworkReply *reply)
{
  // If everything is fine, just exit. The service layer will handle parsing the data.
  if (reply->error() == QNetworkReply::NoError)
  {
    return;
  }

  // Extract the standard HTTP status code (e.g., 401, 404, 500)
  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

  // Handle standard global errors that affect the whole app
  if (statusCode == 401)
  {
    // JWT is missing or expired. Tell the UI to boot the user back to the login screen.
    emit globalErrorOccurred(401, "Session expired. Please log in again.");
    clearAuthToken();
  }
  else if (statusCode >= 500)
  {
    // The backend crashed or threw an exception
    emit globalErrorOccurred(statusCode, "Server error. Please try again later.");
  }

  // Note: We don't delete the reply object here using reply->deleteLater().
  // The service layer still needs to read it to handle specific errors (like a 400 Bad Request).
}
