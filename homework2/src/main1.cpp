#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

class Polynomial;

// 項目類別：存放單一項的係數與指數
class Term {
    friend class Polynomial;
    friend ostream& operator<<(ostream& os, const Polynomial& poly);
    friend istream& operator>>(istream& is, Polynomial& poly);
private:
    float coef; // 係數 
    int exp;    // 指數 
};

// 多項式類別
class Polynomial {
private:
    Term *termArray;    // 動態陣列，放非零項 
    int capacity;       // 陣列初始容量 
    int terms;          // 目前存入的非零項個數 

public:
    // 建構子 
    Polynomial() : capacity(4), terms(0) {
        termArray = new Term[capacity];
    }

    // 解構子 
    ~Polynomial() {
        delete[] termArray;
    }

    // 複製建構子
    Polynomial(const Polynomial &b) {
        capacity = b.capacity;
        terms = b.terms;
        termArray = new Term[capacity];
        for (int i = 0; i < terms; i++) {
            termArray[i] = b.termArray[i];
        }
    }

    // 複製指派運算子
    Polynomial& operator=(const Polynomial &b) {
        if (this != &b) {
            delete[] termArray; // 釋放舊記憶體
            capacity = b.capacity;
            terms = b.terms;
            termArray = new Term[capacity]; // 配置新記憶體
            for (int i = 0; i < terms; i++) {
                termArray[i] = b.termArray[i];
            }
        }
        return *this;
    }

    // 函式宣告
    void newTerm(const float theCoef, const int theExp);
    void SortAndClean(); // 負責排序、合併同類項並剔除 0 係數項
    Polynomial Add(const Polynomial &b);
    Polynomial Mult(const Polynomial &b);
    float Eval(float x);

    // 運算子多載 friend 宣告 
    friend ostream& operator<<(ostream& os, const Polynomial& poly);
    friend istream& operator>>(istream& is, Polynomial& poly);
};

// 新增項至動態陣列中
void Polynomial::newTerm(const float theCoef, const int theExp) {
    if (theCoef == 0) return;
    if (terms == capacity) {
        capacity *= 2;
        Term *temp = new Term[capacity];
        copy(termArray, termArray + terms, temp);
        delete[] termArray;
        termArray = temp;
    }
    termArray[terms].coef = theCoef;
    termArray[terms++].exp = theExp;
}

// 精準排序與零項清理機制
void Polynomial::SortAndClean() {
    // 1. 依指數從大到小進行氣泡排序
    for (int i = 0; i < terms; i++) {
        for (int j = i + 1; j < terms; j++) {
            if (termArray[i].exp < termArray[j].exp) {
                swap(termArray[i], termArray[j]);
            }
        }
    }
    // 2. 原地合併同類項並剔除係數為 0 的項
    int k = 0;
    for (int i = 1; i < terms; i++) {
        if (termArray[k].exp == termArray[i].exp) {
            termArray[k].coef += termArray[i].coef;
        } else {
            if (termArray[k].coef != 0) k++;
            termArray[k] = termArray[i];
        }
    }
    if (terms > 0 && termArray[k].coef != 0) k++;
    terms = k;
}

// 多項式相加
Polynomial Polynomial::Add(const Polynomial &b) {
    Polynomial c;
    int aPos = 0, bPos = 0;
    while ((aPos < terms) && (bPos < b.terms)) {
        if (termArray[aPos].exp == b.termArray[bPos].exp) {
            float t = termArray[aPos].coef + b.termArray[bPos].coef;
            if (t) c.newTerm(t, termArray[aPos].exp);
            aPos++; bPos++;
        }
        else if (termArray[aPos].exp < b.termArray[bPos].exp) {
            c.newTerm(b.termArray[bPos].coef, b.termArray[bPos].exp);
            bPos++;
        }
        else {
            c.newTerm(termArray[aPos].coef, termArray[aPos].exp);
            aPos++;
        }
    }
    for (; aPos < terms; aPos++) c.newTerm(termArray[aPos].coef, termArray[aPos].exp);
    for (; bPos < b.terms; bPos++) c.newTerm(b.termArray[bPos].coef, b.termArray[bPos].exp);
    return c;
}

// 多項式相乘
Polynomial Polynomial::Mult(const Polynomial &b) {
    Polynomial c; 
    for (int i = 0; i < terms; i++) {
        for (int j = 0; j < b.terms; j++) {
            float coef = termArray[i].coef * b.termArray[j].coef;
            int exp = termArray[i].exp + b.termArray[j].exp;
            
            bool found = false;
            for (int k = 0; k < c.terms; k++) {
                if (c.termArray[k].exp == exp) {
                    c.termArray[k].coef += coef;
                    found = true;
                    break;
                }
            }
            if (!found) {
                c.newTerm(coef, exp);
            }
        }
    }
    c.SortAndClean(); // 乘法完畢後全面重新排序並清理同類項
    return c;
}

// 代入 x 計算值
float Polynomial::Eval(float x) {
    float ans = 0;
    for (int i = 0; i < terms; i++) {
        ans += termArray[i].coef * pow(x, termArray[i].exp);
    }
    return ans;
}

// 輸入運算子多載
istream& operator>>(istream& is, Polynomial& poly) {
    string line;
    if (getline(is >> ws, line)) {
        stringstream ss(line);
        float coef;
        int exp;
        while (ss >> coef >> exp) {
            poly.newTerm(coef, exp);
        }
    }
    poly.SortAndClean(); // 輸入第一時間即完成排序，確保 Add() 運算基礎正確
    return is;
}

// 輸出運算子多載
ostream& operator<<(ostream& os, const Polynomial& poly) {
    if (poly.terms == 0) {
        os << "0";
        return os;
    }

    for (int i = 0; i < poly.terms; i++) {
        float c = poly.termArray[i].coef;
        int e = poly.termArray[i].exp;

        if (i == 0) {
            if (c < 0) os << "-";
        } else {
            if (c > 0) os << " + ";
            else os << " - ";
        }

        float absC = fabs(c);
        
        if (e == 0) {
            os << absC;
        } else if (e == 1) {
            if (absC != 1) os << absC;
            os << "X";
        } else {
            if (absC != 1) os << absC;
            os << "X^" << e;
        }
    }
    return os;
}

int main() {
    Polynomial p1, p2;

    cout << "=== 輸入多項式 P1 (輸入 係數 指數 ... 按 Enter 結束) ===\n";
    cin >> p1;

    cout << "\n=== 輸入多項式 P2 (輸入 係數 指數 ... 按 Enter 結束) ===\n";  
    cin >> p2;

    cout << "\n-----------------------------------" << endl;
    cout << "P1 = " << p1 << endl;
    cout << "P2 = " << p2 << endl;
    cout << "-----------------------------------" << endl;

    int choice;
    do {
        cout << "\n==============================\n請選擇要執行的項目：\n";
        cout << "1. 多項式相加 (P1 + P2)\n";
        cout << "2. 多項式相乘 (P1 * P2)\n";
        cout << "3. 代入 x 計算值\n";
        cout << "0. 結束程式\n";
        cout << "請輸入選項 (0-3): ";
        cin >> choice;

        switch (choice) {
            case 1: {
                Polynomial sum = p1.Add(p2);
                cout << "\n>>> P1 + P2 = " << sum << "\n";
                break;
            }
            case 2: {
                Polynomial prod = p1.Mult(p2);
                cout << "\n>>> P1 * P2 = " << prod << "\n";
                break;
            }
            case 3: {
                float x;
                cout << "\n請輸入要代入的x值: ";
                cin >> x;
                cout << "P1(" << x << ") = " << p1.Eval(x) << "\n";
                cout << "P2(" << x << ") = " << p2.Eval(x) << "\n";
                break;
            }
            case 0:
                cout << "\n程式已結束。\n";
                break;
            default:
                cout << "\n無效的選項，請重新選擇\n";
                break;
        }
    } while (choice != 0);

    return 0;
}
