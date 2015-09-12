#include "metadatadialog.h"

MetadataDialog::MetadataDialog(QWidget *parent, const QMap<QString,QVariant> &metadata) :
    QDialog(parent)
{
    init();

    display(metadata);
}

MetadataDialog::MetadataDialog(QWidget *parent, MafwSourceAdapter *mafwSource, const QString &objectId) :
    QDialog(parent),
    objectId(objectId)
{
    init();

    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);

    connect(mafwSource, SIGNAL(metadataResult(QString,GHashTable*,QString)), this, SLOT(onMetadataReceived(QString,GHashTable*)));
    mafwSource->getMetadata(objectId, MAFW_SOURCE_ALL_KEYS);
}

MetadataDialog::~MetadataDialog()
{
    delete ui;
}

void MetadataDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void MetadataDialog::init()
{
    ui = new Ui::MetadataDialog;
    ui->setupUi(this);

    QHeaderView *headerView = ui->metadataTable->horizontalHeader();
    headerView->setResizeMode(0, QHeaderView::ResizeToContents);
    headerView->setResizeMode(1, QHeaderView::Stretch);
    connect(ui->metadataTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
}

void MetadataDialog::display(const QMap<QString,QVariant> &metadata) {
    ui->metadataTable->setRowCount(metadata.size());

    int row = 0;
    for (QMap<QString,QVariant>::const_iterator i = metadata.begin(); i != metadata.end(); ++i, ++row) {
        QVariant value = i.value();
        // Seconds
        if (i.key() == MAFW_METADATA_KEY_DURATION || i.key() == MAFW_METADATA_KEY_PAUSED_POSITION) {
            value = mmss_len(value.toInt());
        }
        // Bytes
        else if (i.key() == MAFW_METADATA_KEY_FILESIZE) {
            value = sizeString(value.toInt());
        }
        // Timestamp
        else if (i.key() == MAFW_METADATA_KEY_ADDED || i.key() == MAFW_METADATA_KEY_MODIFIED || i.key() == MAFW_METADATA_KEY_LAST_PLAYED) {
            value = QLocale().toString(QDateTime::fromTime_t(value.toInt()));
        }
        // Timestamp, show in UTC
        else if (i.key() == MAFW_METADATA_KEY_YEAR) {
            value = QLocale().toString(QDateTime::fromTime_t(value.toInt()).toUTC().date());
        }

        ui->metadataTable->setItem(row, 0, new QTableWidgetItem(i.key()));
        ui->metadataTable->setItem(row, 1, new QTableWidgetItem(value.toString()));
    }
}

QString MetadataDialog::sizeString(double size)
{
    const int K = 1024;
    const int M = 1024 * K;
    const int G = 1024 * M;

    if (size >= G) {
        return QString("%1 GiB").arg(size/G, 0, 'f', 1);
    } else if (size >= M) {
        return QString("%1 MiB").arg(size/M, 0, 'f', 1);
    } else if (size >= K) {
        return QString("%1 KiB").arg(size/K, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(size);
    }
}

void MetadataDialog::onMetadataReceived(const QString &objectId, GHashTable *metadata)
{
    if (objectId != this->objectId) return;

    disconnect(this->sender(), SIGNAL(metadataResult(QString,GHashTable*,QString)), this, SLOT(onMetadataReceived(QString,GHashTable*)));

    QMap<QString,QVariant> qMetadata;

    GList *keys = g_hash_table_get_keys(metadata);

    for (GList *key = keys; key; key = key->next)
        qMetadata[(const char *) key->data] = MafwUtils::toQVariant(mafw_metadata_first(metadata, (const char *) key->data));

    g_list_free(keys);

    display(qMetadata);

    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
}

void MetadataDialog::onContextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Copy"), this, SLOT(copyCurrentItem()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void MetadataDialog::copyCurrentItem()
{
    QApplication::clipboard()->setText(ui->metadataTable->currentItem()->text());
}
