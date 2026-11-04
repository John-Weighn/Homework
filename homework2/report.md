# 41443147

作業二：多項式（Polynomial）動態陣列實作報告

## 解題說明

1. **實作 Polynomial 類別的 ADT**：包含作業規範中的 `termArray`（動態 Term 陣列）、`capacity`（容量）與 `terms`（目前非零項個數）等 private 資料成員。
2. **串流運算子多載**：實作 `<<` 與 `>>` 運算子，提供使用者直覺且符合 C++ 標準格式的輸入與輸出介面。
3. **核心功能實作**：精準實作多項式的加法（`Add`）、乘法（`Mult`）以及代入求值（`Eval`）。

### 解題策略與優化

#### 1. 自動排序與同類項合併（防呆機制）
原先的雙指標加法（`Add`）演算法強烈依賴「多項式必須依指數由大到小排序」的前提。為此，本程式導入了 `SortAndClean()` 核心函式：
- **輸入防呆**：使用者輸入時不需按照指數順序（例如輸入 `3 1 5 3`），程式會自動以氣泡排序法將其由大到小重新排列。
- **乘法合併**：多項式相乘後會產生雜亂的指數項，透過此機制能確保結果多項式自動完成同類項合併，並**剔除係數相加後消去為 0 的無效項**。

#### 2. 使用動態記憶體配置
- 採用 `Term` 結構體動態陣列存取非零項。初始容量（`capacity`）設定為 4。
- 當動態空間不足（`terms == capacity`）時，自動觸發擴容機制將容量翻倍（`capacity *= 2`），並利用 `std::copy` 將舊資料無縫移轉至新記憶體，隨後安全釋放舊陣列，達到空間的高效利用。

#### 3. 智慧型輸入與輸出處理
- **輸入處理**：利用 `is >> ws` 自動清除前導空白與換行，配合 `std::getline` 讀取整行文字並丟入 `std::stringstream`。只要使用者輸入成對的「係數」與「指數」，**直接按下 Enter 鍵即可瀟灑結束該多項式的輸入**，極具人性化。
- **數學標準排版輸出**：優化 `<<` 運算子，首項負號自動前置，中間項的正負號與數字間保持優雅留白（如 `3X^2 - 4X + 5`）。此外，當係數為 `1` 或 `-1` 時，程式會聰明地隱藏數字（顯示 `X^2` 而非 `1X^2`），完全符合人類的數學書寫習慣。

---

## 程式實作

完整的 C++ 原始碼如下：

```cpp
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
```

---

## 效能分析

| 運算或函式 | 時間複雜度 (Time Complexity) | 空間複雜度 (Space Complexity) | 說明 |
| :--- | :--- | :--- | :--- |
| **`newTerm()`** | 平均 $O(1)$ / 最壞 $O(n)$ | $O(n)$ | 陣列容量充足時為 $O(1)$。若空間全滿觸發擴容機制，則需耗費 $O(n)$ 時間進行複製。 |
| **`SortAndClean()`**| $O(n^2)$ | $O(1)$ | 採用氣泡排序對非零項進行排序，並在原地線性掃描完成同類項合併。 |
| **`Add()`** | $O(m + n)$ | $O(m + n)$ | $m, n$ 分別為兩多項式的非零項數量。由於兩者皆已排序，採雙指標線性對齊掃描，效率極高。 |
| **`Mult()`** | $O(m \times n \times k)$ | $O(m \times n)$ | 展開雙重迴圈耗費 $O(m \times n)$，每次新增需在最長為 $k$ 的陣列中搜尋同類項。最後透過 `SortAndClean()` 確保整體有序性。 |
| **`Eval()`** | $O(n \log E)$ | $O(1)$ | $E$ 為多項式的最大指數。時間成本取決於底層 `std::pow` 的快速冪二進位指數化計算。 |
| **`operator>>`** | $O(n^2)$ | $O(n)$ | $n$ 為輸入行內解析出的總項數。資料流讀取完畢後隨即呼叫 `SortAndClean()`。 |
| **`operator<<`** | $O(n)$ | $O(1)$ | 線性掃描陣列，一次性格式化印出所有合規的非零項。 |

---

## 測試與驗證

### 測試案例

| 測試案例 | 輸入多項式 P1 | 輸入多項式 P2 | $x$ 代入值 | `Add()` 輸出結果 | `Mult()` 輸出結果 | `Eval()` 輸出結果 |
| :---: | :--- | :--- | :---: | :--- | :--- | :--- |
| **測試一** | `3 2 1 1` *(3X²+X)* | `2 1 3 0` *(2X+3)* | 2 | `3X^2 + 3X + 3` | `6X^3 + 11X^2 + 3X` | P1=14, P2=7 |
| **測試二** | `2 5 1 0` *(2X⁵+1)* | `1 1 2 0` *(X+2)* | 1 | `2X^5 + X + 3` | `2X^6 + 4X^5 + X + 2` | P1=3, P2=3 |
| **測試三** | `-1 3` *(-X³)* | `-2 1` *(-2X)* | 3 | `-X^3 - 2X` | `2X^4` | P1=-27, P2=-6 |
| **測試四** | `3 3 2 2 1 0` | `2 1 1 0` | 0 | `3X^3 + 2X^2 + 2X + 2`| `6X^4 + 7X^3 + 2X^2 + 2X + 1`| P1=1, P2=1 |

### 編譯與執行指令

```shell
$ g++ -std=c++11 -O2 Polynomial.cpp -o Polynomial.exe
$ ./Polynomial.exe
```

---

## 申論及開發報告

### 使用陣列與動態記憶體配置的原因

