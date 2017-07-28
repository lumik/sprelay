/***************************************************************************
**                                                                        **
**  Controlling interface for K8090 8-Channel Relay Card from Velleman    **
**  through usb using virtual serial port in Qt.                          **
**  Copyright (C) 2017 Jakub Klener                                       **
**                                                                        **
**  This file is part of SpRelay application.                             **
**                                                                        **
**  You can redistribute it and/or modify it under the terms of the       **
**  3-Clause BSD License as published by the Open Source Initiative.      **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          **
**  3-Clause BSD License for more details.                                **
**                                                                        **
**  You should have received a copy of the 3-Clause BSD License along     **
**  with this program.                                                    **
**  If not, see https://opensource.org/licenses/                          **
**                                                                        **
****************************************************************************/

#include <QtTest/QtTest>
#include <QDebug>

// dirty trick which enables us to test private methods. Think of something
// else.
#define private public
#include "k8090.h"

class TestK8090: public QObject
{
    Q_OBJECT
private slots:  // NOLINT(whitespace/indent)
    void hexToByte();
    void lowByt (); // we are going to test saving numbers into to separated bytes.
    void highByt();
};
/*!
 * \brief TestK8090::hextoByte()
 */
void TestK8090::hexToByte()
{
    unsigned char *bMsg;
    int n;

    // testing message
    unsigned char nMsg[3] = {0x1, 0xFF, 0xF};
    QString msg = "01 FF 0F";

    K8090::hexToByte(&bMsg, &n, msg);

    bool ok = 1;
    for (int i = 0; i < n; i++) {
        if (bMsg[i] != nMsg[i]) {
            ok = 0;
        }
    }

    delete[] bMsg;  // hexToByte created new variable, don't forget to delete it

    QCOMPARE(ok, true);
}
/*!
 * \brief TestK8090::lowByt
 */
void TestK8090::lowByt ()
{  //our testing value will be 41 (0x29 in hexdec., 0b101001)
    bool ok = 0;
    unsigned char byt;
    byt = (41)&(0xFF);
    if (byt== 0x29)
    {
      ok=1;
    }
    QCOMPARE(ok,true);
}
/*!
 * \brief TestK8090::highByt
 */
void TestK8090::highByt ()
{  //our testing value will be 41 (0x29 in hexdec., 0b101001)
    bool ok = 0;
    unsigned char byt;
    byt = (10496>>8)&(0xFF); //mistake in testing value
    if (byt == 0x29)
    {
      ok=1;
      qDebug() << "it's okey.";
    }
    else
        qDebug()<<"Something is wrong";
    QCOMPARE(ok,true);
}


QTEST_MAIN(TestK8090)
#include "testk8090.moc"
