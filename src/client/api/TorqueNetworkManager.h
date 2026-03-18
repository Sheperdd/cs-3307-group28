#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QByteArray>

class TorqueNetworkManager : public QObject
{

Q_OBJECT // requried for signals/slots

    public :
    // We pass a parent pointer here (standard Qt memory management),
    // but we will also pass this manager's pointer to our service classes (Dependency Injection)
    explicit TorqueNetworkManager(QObject *parent = nullptr);

  // Methods to manage the JWT token for authenticated requests
  void setAuthToken(const QString &token);
  void clearAuthToken();

  // Core HTTP actions. They return QNetworkReply pointer so the
  // service layer can read the specific JSON response when it's ready.
  QNetworkReply *get(const QString &endpoint);
  QNetworkReply *post(const QString &endpoint, const QByteArray &body);
  QNetworkReply *patch(const QString &endpoint, const QByteArray &body);
  QNetworkReply *deleteResource(const QString &endpoint); // 'delete' is a keyword

signals:
  // broadcasts major issues to the UI layer, like auth failures or server errors
  void globalErrorOccurred(int statusCode, const QString &message);

private slots:
  // Internal slot to handle all network replies and emit global errors if needed
  void onReplyFinished(QNetworkReply *reply);

private:
  QNetworkAccessManager *manager; // actual Qt engine doing the networking
  QString baseUrl;                // base URL for all API calls, for now http://localhost:6967
  QString authToken;              // the current JWT token for authenticated requests

  // Helper function to attach the URL, JSON headers, and JWT to every request automatically
  QNetworkRequest createRequest(const QString &endpoint);
};
