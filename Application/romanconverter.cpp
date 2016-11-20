#include "romanconverter.h"
#include <QMap>

QString int2roman(int n) {
    QString ans;
    QString M[] = {"","M","MM","MMM"};
    QString C[] = {"","C","CC","CCC","CD","D","DC","DCC","DCCC","CM"};
    QString X[] = {"","X","XX","XXX","XL","L","LX","LXX","LXXX","XC"};
    QString I[] = {"","I","II","III","IV","V","VI","VII","VIII","IX"};
    ans = M[n/1000]+C[(n%1000)/100]+X[(n%100)/10]+I[(n%10)];
    return ans;
}

int getValue(QChar c) {
    switch (c.toLatin1()) {
    case 'I':return 1;
    case 'V':return 5;
    case 'X':return 10;
    case 'L':return 50;
    case 'C':return 100;
    case 'D':return 500;
    case 'M':return 1000;
    }
    return 0;
}

int roman2int(QString s) {
    int sum=0;
    QChar prev = '%';
    for(int i = s.size()-1; i >= 0; i--) {
        if(getValue(s[i])<sum && s[i] != prev) {
            sum -= getValue(s[i]);
        } else {
            sum += getValue(s[i]);
        }
        prev = s[i];
    }
    return sum;
}
