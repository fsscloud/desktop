/*
 * Copyright (C) by Dominik Schmidt <dev@dominik-schmidt.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */


#ifndef SOCKETAPI_H
#define SOCKETAPI_H

#include "syncfileitem.h"
#include "syncfilestatus.h"
#include "sharedialog.h" // for the ShareDialogStartPage
#include "common/syncjournalfilerecord.h"

#if defined(Q_OS_MAC)
#include "socketapisocket_mac.h"
#else
#include <QLocalServer>
using SocketApiServer = QLocalServer;
#endif

class QUrl;
class QLocalSocket;
class QStringList;

namespace OCC {

class SyncFileStatus;
class Folder;
class SocketListener;
class DirectEditor;

/**
 * @brief The SocketApi class
 * @ingroup gui
 */
class SocketApi : public QObject
{
    Q_OBJECT

public:
    explicit SocketApi(QObject *parent = nullptr);
    virtual ~SocketApi();

public slots:
    void slotUpdateFolderView(Folder *f);
    void slotUnregisterPath(const QString &alias);
    void slotRegisterPath(const QString &alias);
    void broadcastStatusPushMessage(const QString &systemPath, SyncFileStatus fileStatus);

signals:
    void shareCommandReceived(const QString &sharePath, const QString &localPath, ShareDialogStartPage startPage);

private slots:
    void slotNewConnection();
    void onLostConnection();
    void slotSocketDestroyed(QObject *obj);
    void slotReadSocket();

    static void copyUrlToClipboard(const QString &link);
    static void emailPrivateLink(const QString &link);
    static void openPrivateLink(const QString &link);

private:
    // Helper structure for getting information on a file
    // based on its local path - used for nearly all remote
    // actions.
    struct FileData
    {
        static FileData get(const QString &localFile);
        SyncFileStatus syncFileStatus() const;
        SyncJournalFileRecord journalRecord() const;

        Folder *folder;
        QString localPath;
        QString folderRelativePath;
        QString accountRelativePath;
    };

    void broadcastMessage(const QString &msg, bool doWait = false);

    // opens share dialog, sends reply
    void processShareRequest(const QString &localFile, SocketListener *listener, ShareDialogStartPage startPage);

    Q_INVOKABLE void command_RETRIEVE_FOLDER_STATUS(const QString &argument, SocketListener *listener);
    Q_INVOKABLE void command_RETRIEVE_FILE_STATUS(const QString &argument, SocketListener *listener);

    Q_INVOKABLE void command_VERSION(const QString &argument, SocketListener *listener);

    Q_INVOKABLE void command_SHARE_MENU_TITLE(const QString &argument, SocketListener *listener);

    // The context menu actions
    Q_INVOKABLE void command_SHARE(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_MANAGE_PUBLIC_LINKS(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_COPY_PUBLIC_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_COPY_PRIVATE_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_EMAIL_PRIVATE_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_OPEN_PRIVATE_LINK(const QString &localFile, SocketListener *listener);

    // Windows Shell / Explorer pinning fallbacks, see issue: https://github.com/nextcloud/desktop/issues/1599
#ifdef Q_OS_WIN
    Q_INVOKABLE void command_COPYASPATH(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_OPENNEWWINDOW(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_OPEN(const QString &localFile, SocketListener *listener);
#endif

    // Fetch the private link and call targetFun
    void fetchPrivateLinkUrlHelper(const QString &localFile, const std::function<void(const QString &url)> &targetFun);

    /** Sends translated/branded strings that may be useful to the integration */
    Q_INVOKABLE void command_GET_STRINGS(const QString &argument, SocketListener *listener);

    // Sends the context menu options relating to sharing to listener
    void sendSharingContextMenuOptions(const FileData &fileData, SocketListener *listener);

    /** Send the list of menu item. (added in version 1.1)
     * argument is a list of files for which the menu should be shown, separated by '\x1e'
     * Reply with  GET_MENU_ITEMS:BEGIN
     * followed by several MENU_ITEM:[Action]:[flag]:[Text]
     * If flag contains 'd', the menu should be disabled
     * and ends with GET_MENU_ITEMS:END
     */
    Q_INVOKABLE void command_GET_MENU_ITEMS(const QString &argument, SocketListener *listener);

    /// Direct Editing
    Q_INVOKABLE void command_EDIT(const QString &localFile, SocketListener *listener);
    DirectEditor* getDirectEditorForLocalFile(const QString &localFile);

    QString buildRegisterPathMessage(const QString &path);

    QSet<QString> _registeredAliases;
    QList<SocketListener> _listeners;
    SocketApiServer _localServer;
};
}
#endif // SOCKETAPI_H
