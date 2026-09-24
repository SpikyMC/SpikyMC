#include <QTest>

#include "ui/themes/CatPack.h"

class CatPackTest : public QObject {
    Q_OBJECT
   private slots:
    void test_basicCatPack()
    {
        BasicCatPack pack("spiky", "Spiky");
        QCOMPARE(pack.id(), "spiky");
        QCOMPARE(pack.name(), "Spiky");
        QCOMPARE(pack.path(), ":/backgrounds/spiky");
    }

    void test_basicCatPack_implicitName()
    {
        BasicCatPack pack("spiky");
        QCOMPARE(pack.id(), "spiky");
        QCOMPARE(pack.name(), "spiky");
        QCOMPARE(pack.path(), ":/backgrounds/spiky");
    }
};

QTEST_GUILESS_MAIN(CatPackTest)

#include "CatPack_test.moc"