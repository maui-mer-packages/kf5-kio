/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_JOB_BASE_H
#define KIO_JOB_BASE_H

#include <kcompositejob.h>
#include <kio/metadata.h>

namespace KIO
{

class JobUiDelegateExtension;

class JobPrivate;
/**
 * The base class for all jobs.
 * For all jobs created in an application, the code looks like
 *
 * \code
 *   KIO::Job * job = KIO::someoperation( some parameters );
 *   connect( job, SIGNAL( result( KJob * ) ),
 *            this, SLOT( slotResult( KJob * ) ) );
 * \endcode
 *   (other connects, specific to the job)
 *
 * And slotResult is usually at least:
 *
 * \code
 *  if ( job->error() )
 *      job->ui()->showErrorMessage();
 * \endcode
 * @see KIO::Scheduler
 */
class KIOCORE_EXPORT Job : public KCompositeJob
{
    Q_OBJECT

protected:
    Job();
    Job(JobPrivate &dd);

public:
    virtual ~Job();
    void start() {} // Since KIO autostarts its jobs

    /**
     * Retrieves the UI delegate of this job.
     *
     * @deprecated since 5.0, can now be replaced with uiDelegate()
     *
     * @return the delegate used by the job to communicate with the UI
     */
    KJobUiDelegate *ui() const;

    /**
     * Retrieves the UI delegate extension used by this job.
     * @since 5.0
     */
    JobUiDelegateExtension *uiDelegateExtension() const;

    /**
     * Sets the UI delegate extension to be used by this job.
     * The default UI delegate extension is KIO::defaultJobUiDelegateExtension()
     */
    void setUiDelegateExtension(JobUiDelegateExtension *extension);

protected:
    /**
     * Abort this job.
     * This kills all subjobs and deletes the job.
     *
     */
    virtual bool doKill();

    /**
     * Suspend this job
     * @see resume
     */
    virtual bool doSuspend();

    /**
     * Resume this job
     * @see suspend
     */
    virtual bool doResume();

public:
    /**
     * Converts an error code and a non-i18n error message into an
     * error message in the current language. The low level (non-i18n)
     * error message (usually a url) is put into the translated error
     * message using %1.
     *
     * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
     * \code
     *   i18n( "Could not read\n%1" ).arg( errortext );
     * \endcode
     * Use this to display the error yourself, but for a dialog box
     * use ui()->showErrorMessage(). Do not call it if error()
     * is not 0.
     * @return the error message and if there is no error, a message
     *         telling the user that the app is broken, so check with
     *         error() whether there is an error
     */
    QString errorString() const;

    /**
     * Converts an error code and a non-i18n error message into i18n
     * strings suitable for presentation in a detailed error message box.
     *
     * @param reqUrl the request URL that generated this error message
     * @param method the method that generated this error message
     * (unimplemented)
     * @return the following strings: caption, error + description,
     *         causes+solutions
     */
    QStringList detailedErrorStrings(const QUrl *reqUrl = 0L,
                                     int method = -1) const;

    /**
     * Set the parent Job.
     * One example use of this is when FileCopyJob calls RenameDialog::open,
     * it must pass the correct progress ID of the parent CopyJob
     * (to hide the progress dialog).
     * You can set the parent job only once. By default a job does not
     * have a parent job.
     * @param parentJob the new parent job
     */
    void setParentJob(Job *parentJob);

    /**
     * Returns the parent job, if there is one.
     * @return the parent job, or 0 if there is none
     * @see setParentJob
     */
    Job *parentJob() const;

    /**
     * Set meta data to be sent to the slave, replacing existing
     * meta data.
     * @param metaData the meta data to set
     * @see addMetaData()
     * @see mergeMetaData()
     */
    void setMetaData(const KIO::MetaData &metaData);

    /**
     * Add key/value pair to the meta data that is sent to the slave.
     * @param key the key of the meta data
     * @param value the value of the meta data
     * @see setMetaData()
     * @see mergeMetaData()
     */
    void addMetaData(const QString &key, const QString &value);

    /**
     * Add key/value pairs to the meta data that is sent to the slave.
     * If a certain key already existed, it will be overridden.
     * @param values the meta data to add
     * @see setMetaData()
     * @see mergeMetaData()
     */
    void addMetaData(const QMap<QString, QString> &values);

    /**
     * Add key/value pairs to the meta data that is sent to the slave.
     * If a certain key already existed, it will remain unchanged.
     * @param values the meta data to merge
     * @see setMetaData()
     * @see addMetaData()
     */
    void mergeMetaData(const QMap<QString, QString> &values);

    /**
     * @internal. For the scheduler. Do not use.
     */
    MetaData outgoingMetaData() const;

    /**
     * Get meta data received from the slave.
     * (Valid when first data is received and/or slave is finished)
     * @return the job's meta data
     */
    MetaData metaData() const;

    /**
     * Query meta data received from the slave.
     * (Valid when first data is received and/or slave is finished)
     * @param key the key of the meta data to retrieve
     * @return the value of the meta data, or QString() if the
     *         @p key does not exist
     */
    QString queryMetaData(const QString &key);

protected:

Q_SIGNALS:
    /**
     * @deprecated. Don't use !
     * Emitted when the job is canceled.
     * Signal result() is emitted as well, and error() is,
     * in this case, ERR_USER_CANCELED.
     * @param job the job that emitted this signal
     */
    void canceled(KJob *job);

    /**
     * Emitted when the slave successfully connected to the host.
     * There is no guarantee the slave will send this, and this is
     * currently unused (in the applications).
     * @param job the job that emitted this signal
     */
    void connected(KIO::Job *job);

protected:
    /**
     * Add a job that has to be finished before a result
     * is emitted. This has obviously to be called before
     * the finish signal is emitted by the slave.
     *
     * @param job the subjob to add
     */
    virtual bool addSubjob(KJob *job);

    /**
     * Mark a sub job as being done.
     *
     * KDE4 change: this doesn't terminate the parent job anymore, call emitResult to do that.
     *
     * @param job the subjob to remove
     */
    virtual bool removeSubjob(KJob *job);

protected:
    JobPrivate *const d_ptr;

private:
    /**
     * Forward signal from subjob.
     * @param job the subjob
     * @param speed the speed in bytes/s
     * @see speed()
     */
    Q_PRIVATE_SLOT(d_func(), void slotSpeed(KJob *job, unsigned long speed))
    Q_DECLARE_PRIVATE(Job)
};

/**
 * Flags for the job properties.
 * Not all flags are supported in all cases. Please see documentation of
 * the calling function!
 */
enum JobFlag {
    /**
     * Show the progress info GUI, no Resume and no Overwrite
     */
    DefaultFlags = 0,

    /**
     * Hide progress information dialog, i.e. don't show a GUI.
     */
    HideProgressInfo = 1,

    /**
     * When set, automatically append to the destination file if it exists already.
     * WARNING: this is NOT the builtin support for offering the user to resume a previous
     * partial download. The Resume option is much less used, it allows to append
     * to an existing file.
     * This is used by KIO::put(), KIO::file_copy(), KIO::file_move().
     */
    Resume = 2,

    /**
     * When set, automatically overwrite the destination if it exists already.
     * This is used by KIO::rename(), KIO::put(), KIO::file_copy(), KIO::file_move(), KIO::symlink().
     * Otherwise the operation will fail with ERR_FILE_ALREADY_EXIST or ERR_DIR_ALREADY_EXIST.
     */
    Overwrite = 4
};
Q_DECLARE_FLAGS(JobFlags, JobFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(JobFlags)

enum LoadType { Reload, NoReload };

}

#endif