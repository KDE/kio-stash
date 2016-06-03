#ifndef KIO_STAGING_H
#define KIO_STAGING_H

#include <kio/forwardingslavebase.h>

class Staging : public KIO::ForwardingSlaveBase
{
    Q_OBJECT
public:
    Staging(const QByteArray &pool, const QByteArray &app);
    ~Staging(){};
    void listDir(const QUrl &url) Q_DECL_OVERRIDE;
    //void prepareUDSEntry(KIO::UDSEntry &entry, bool listing=false) const;

    void rename(const QUrl &, const QUrl &, KIO::JobFlags flags) Q_DECL_OVERRIDE;
protected:
    bool rewriteUrl(const QUrl &url, QUrl &newUrl) Q_DECL_OVERRIDE;
};

#endif
