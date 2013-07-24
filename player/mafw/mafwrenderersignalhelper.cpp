/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Nicolai Hess

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "mafwrenderersignalhelper.h"

#ifdef MAFW_WORKAROUNDS
int MafwRendererSignalHelper::play_retries = 0;
#endif

void MafwRendererSignalHelper::play_playback_cb(MafwRenderer*,
                                                gpointer user_data,
                                                const GError* error)
{
    QString qerror;
#ifdef LIBPLAYBACK_FULL
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalPlay(qerror);
#else
    MafwRendererAdapter *adapter = static_cast<MafwRendererAdapter*>(user_data);

    if (error) {
        qerror = QString(error->message);
        qDebug() << qerror;
    } else {
        if (adapter->playback && adapter->compatiblePlayback)
            pb_playback_req_state(adapter->playback, PB_STATE_PLAY, adapter->playback_state_req_callback, NULL);
    }

    emit adapter->signalPlay(qerror);
#endif

#ifdef MAFW_WORKAROUNDS
    // MAFW behaves inconsistently when it comes to assigning items to
    // a renderer. It can end up without any media to play despite the playlist
    // not being empty. gotoIndex() can fix the issue, but next() is probably
    // better due to being shuffle-friendly.
    if (error && error->code == MAFW_RENDERER_ERROR_NO_MEDIA) {
        MafwRendererAdapter* mafwrenderer = static_cast<MafwRendererAdapter*>(user_data);
        if (mafwrenderer->playlist->getSize()) {
            if (play_retries < 5) {
                qDebug() << "Trying to recover from MAFW_RENDERER_ERROR_NO_MEDIA";
                ++play_retries;
                mafwrenderer->next();
                mafwrenderer->play();
            } else {
                qDebug() << "Giving up on MAFW_RENDERER_ERROR_NO_MEDIA";
            }
            return;
        }
    }
    play_retries = 0;
#endif
}

void MafwRendererSignalHelper::play_uri_playback_cb(MafwRenderer*,
                                                    gpointer user_data,
                                                    const GError* error)
{
    QString qerror;
#ifdef LIBPLAYBACK_FULL
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalPlayURI(qerror);
#else
    MafwRendererAdapter *adapter = static_cast<MafwRendererAdapter*>(user_data);

    if (error) {
        qerror = QString(error->message);
        qDebug() << qerror;
    } else {
        if (adapter->playback && adapter->compatiblePlayback)
            pb_playback_req_state(adapter->playback, PB_STATE_PLAY, adapter->playback_state_req_callback, NULL);
    }

    emit adapter->signalPlayURI(qerror);
#endif
}

void MafwRendererSignalHelper::play_object_playback_cb(MafwRenderer*,
                                                       gpointer user_data,
                                                       const GError* error)
{
    QString qerror;
#ifdef LIBPLAYBACK_FULL
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalPlayObject(qerror);
#else
    MafwRendererAdapter *adapter = static_cast<MafwRendererAdapter*>(user_data);

    if (error) {
        qerror = QString(error->message);
        qDebug() << qerror;
    } else {
        if (adapter->playback && adapter->compatiblePlayback)
            pb_playback_req_state(adapter->playback, PB_STATE_PLAY, adapter->playback_state_req_callback, NULL);
    }

    emit adapter->signalPlayObject(qerror);
#endif
}

void MafwRendererSignalHelper::stop_playback_cb(MafwRenderer*,
                                                gpointer user_data,
                                                const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalStop(qerror);
}

void MafwRendererSignalHelper::pause_playback_cb(MafwRenderer*,
                                                 gpointer user_data,
                                                 const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalPause(qerror);
}

void MafwRendererSignalHelper::resume_playback_cb(MafwRenderer*,
                                                  gpointer user_data,
                                                  const GError* error)
{
    QString qerror;
#ifdef LIBPLAYBACK_FULL
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalResume(qerror);
#else
    MafwRendererAdapter *adapter = static_cast<MafwRendererAdapter*>(user_data);

    if (error) {
        qerror = QString(error->message);
        qDebug() << qerror;
    } else {
        if (adapter->playback && adapter->compatiblePlayback)
            pb_playback_req_state(adapter->playback, PB_STATE_PLAY, adapter->playback_state_req_callback, NULL);
    }

    emit adapter->signalResume(qerror);
#endif
}

void MafwRendererSignalHelper::get_status_cb(MafwRenderer*,
                                             MafwPlaylist* playlist,
                                             uint index,
                                             MafwPlayState state,
                                             const char* object_id,
                                             gpointer user_data,
                                             const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    emit static_cast<MafwRendererAdapter*>(user_data)->signalGetStatus(playlist, index, state, object_id, qerror);
}

void MafwRendererSignalHelper::next_playback_cb(MafwRenderer*,
                                                gpointer user_data,
                                                const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    emit static_cast<MafwRendererAdapter*>(user_data)->signalNext(qerror);
}

void MafwRendererSignalHelper::previous_playback_cb(MafwRenderer*,
                                                    gpointer user_data,
                                                    const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    emit static_cast<MafwRendererAdapter*>(user_data)->signalPrevious(qerror);
}

void MafwRendererSignalHelper::goto_index_playback_cb(MafwRenderer*,
                                                      gpointer user_data,
                                                      const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    emit static_cast<MafwRendererAdapter*>(user_data)->signalGotoIndex(qerror);
}

void MafwRendererSignalHelper::set_position_cb(MafwRenderer*,
                                               int position,
                                               gpointer user_data,
                                               const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalSetPosition(position, qerror);
}

void MafwRendererSignalHelper::get_position_cb(MafwRenderer*,
                                               int position,
                                               gpointer user_data,
                                               const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
    emit static_cast<MafwRendererAdapter*>(user_data)->signalGetPosition(position, qerror);
}

void MafwRendererSignalHelper::get_current_metadata_cb(MafwRenderer*,
                                                       const gchar* object_id,
                                                       GHashTable* metadata,
                                                       gpointer user_data,
                                                       const GError* error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }

    emit static_cast<MafwRendererAdapter*>(user_data)->signalGetCurrentMetadata(metadata,
                                                                                QString::fromUtf8(object_id), qerror);
}

void MafwRendererSignalHelper::get_property_cb(MafwExtension *,
                                               const gchar *name,
                                               GValue *value,
                                               gpointer user_data,
                                               const GError *error)
{
    QString qerror;
    if(error)
    {
        qerror = QString(error->message);
    }
    if (!qerror.isEmpty())
        qDebug() << qerror;
#ifdef DEBUG
    qDebug() << "MafwRendererSignalHelper::get_property_cb";
#endif
    if (strcmp (name, MAFW_PROPERTY_RENDERER_VOLUME) == 0) {
        int volume = g_value_get_uint (value);
        emit static_cast<MafwRendererAdapter*>(user_data)->signalGetVolume(volume);
    }
}
