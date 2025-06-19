#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <ctime>
#include <algorithm>
#include <iomanip>  
#include <sstream>

using namespace std;

// Enums
enum UmbrellaStatus { NORMAL, LOST, DAMAGED };
enum ReportType { LOST_REPORT, DAMAGED_REPORT };
enum DonationMethod { PICKUP, SHIPPING };

// Classes
struct PaymentCard {
    string cardNumber;
    string expiryDate;
    string cvc;
    string ownerName;
    string pwdFrontTwoDigits;
};

struct Umbrella {
    string id;
    UmbrellaStatus status;
    bool isAvailable;
};

struct Locker {
    string id;
    vector<Umbrella> umbrellas;
};

struct Report {
    string umbrellaId;
    ReportType type;
    string userId;
};

struct Donation {
    int count;
    DonationMethod method;
    string userId;
};

class User {
public:
    string name, phone, id, pwd;
    PaymentCard card;
    Umbrella rentedUmbrella;
    time_t rentTime = 0;
    int seeds = 0;
    bool hasRented = false;

    void rentUmbrella(Umbrella umbrella) {
        rentedUmbrella = umbrella;
        rentTime = time(0);
        hasRented = true;

        char buffer[26];
        ctime_s(buffer, sizeof(buffer), &rentTime);
        cout << "· 대여 시작됨! 우산 ID: " << umbrella.id << " | 시간 측정 시작: " << buffer;
    }

    void returnUmbrella(Locker lockers[], int lockerCount) {
        if (!hasRented) {
            cout << "· 대여 중인 우산이 없습니다.\n";
            return;
        }

        time_t now = time(0);
        int seconds = difftime(now, rentTime);
        if (seconds > 3600) {
            int hours = seconds / 3600;
            int cost = hours * 1000;
            cout << "· 1시간 초과: 자동 결제 시도 중... (" << cost << "원)\n";

            if (processPayment(cost)) {
                cout << "· 결제 성공\n";
            }
            else {
                cout << "[오류] 결제 실패! 카드 정보를 다시 확인해주세요.\n";
                return;
            }
        }
        else {
            cout << "· 우산 반납 완료 (1시간 이내)\n";
        }

        string lockerId;
        cout << "반납할 보관함 ID 입력 (A~E): ";
        cin >> lockerId;

        for (int i = 0; i < lockerCount; ++i) {
            if (lockers[i].id == lockerId) {
                rentedUmbrella.isAvailable = true;
                lockers[i].umbrellas.push_back(rentedUmbrella);
                break;
            }
        }

        hasRented = false;
        rentTime = 0;
        seeds++;
    }

    bool processPayment(int amount) {
        if (card.cardNumber.length() == 16 &&
            card.cvc.length() == 3 &&
            card.expiryDate.length() >= 4 &&
            card.pwdFrontTwoDigits.length() == 2) {
            cout << "결제 성공: " << amount << "원\n";
            return true;
        }

        cout << "결제 실패: 카드 정보 오류\n";
        return false;
    }

    void showRentalStatus() {
        if (!hasRented) {
            cout << "현재 대여 중인 우산이 없습니다.\n";
            return;
        }
        time_t now = time(0);
        int seconds = difftime(now, rentTime);
        int minutes = seconds / 60;
        cout << "현재 대여 중인 우산 ID: " << rentedUmbrella.id << "\n";
        cout << "대여 경과 시간: " << minutes << "분\n";
    }
};

unordered_map<string, User> users;
vector<Report> reports;
vector<Donation> donations;
Locker lockers[5] = { {"A"}, {"B"}, {"C"}, {"D"}, {"E"} };
User* currentUser = nullptr;
bool isAdmin = false;

// Utils
Umbrella* findAvailableUmbrella(string lockerId) {
    for (auto& locker : lockers) {
        if (locker.id == lockerId) {
            sort(locker.umbrellas.begin(), locker.umbrellas.end(), [](const Umbrella& a, const Umbrella& b) {
                return a.id < b.id;
                });
            for (auto& umb : locker.umbrellas) {
                if (umb.status == NORMAL && umb.isAvailable)
                    return &umb;
            }
        }
    }
    return nullptr;
}

void showLockers() {
    for (auto& locker : lockers) {
        int count = 0;
        for (auto& u : locker.umbrellas)
            if (u.status == NORMAL && u.isAvailable) count++;
        cout << locker.id << "보관함 - 우산 수: " << count << endl;
    }
}

bool isExpiryDateValid(const string& expiry) {
    if (expiry.length() != 5 || expiry[2] != '/')
        return false;

    int month, year;
    try {
        month = stoi(expiry.substr(0, 2));
        year = stoi(expiry.substr(3, 2));
    }
    catch (...) {
        return false;
    }

    if (month < 1 || month > 12)
        return false;

    time_t now = time(0);
    tm now_tm;
    localtime_s(&now_tm, &now);


    int currentYear = now_tm.tm_year % 100;
    int currentMonth = now_tm.tm_mon + 1;

    if (year > currentYear) return true;
    else if (year == currentYear && month >= currentMonth) return true;

    return false;
}

void signup() {
    string name, phone, id, pwd, cardNum, expiry, cvc, pwdFront, owner;


    cout << "회원가입 시작 (취소하려면 exit 입력) \n";

    cout << "이름 : ";
    cin >> name;
    if (name == "exit") {
        cout << "회원가입 취소\n";
        return;
    }
    cout << "전화번호 : ";
    cin >> phone;
    if (phone == "exit") {
        cout << "회원가입 취소\n";
        return;
    }
    cout << "아이디 : ";
    cin >> id;
    if (id == "exit") {
        cout << "회원가입 취소\n";
        return;
    }
    if (users.count(id)) {
        cout << "이미 존재하는 아이디입니다.\n";
        return;
    }
    cout << "비밀번호 : ";
    cin >> pwd;
    if (pwd == "exit") {
        cout << "회원가입 취소\n";
        return;
    }

    cout << " [ 결제 정보 등록 ]\n";

    while (true) {
        cout << "카드번호(숫자만 16자리): ";
        cin >> cardNum;
        if (cardNum == "exit") { cout << "회원가입 취소\n"; return; }
        if (cardNum.length() != 16) {
            cout << "[오류] 카드번호를 정확하게 입력하세요.\n";
            continue;
        }
        break;
    }

    // 만료일 입력
    while (true) {
        cout << "만료일(MM/YY): ";
        cin >> expiry;
        if (expiry == "exit") { cout << "회원가입 취소\n"; return; }
        if (!isExpiryDateValid(expiry)) {
            cout << "[오류] 카드 만료일이 현재보다 이전입니다. 다시 입력해주세요.\n";
            continue;
        }
        break;
    }

    // CVC 입력
    while (true) {
        cout << "CVC(3자리): ";
        cin >> cvc;
        if (cvc == "exit") { cout << "회원가입 취소\n"; return; }
        if (cvc.length() != 3) {
            cout << "[오류] CVC를 정확하게 입력하세요.\n";
            continue;
        }
        break;
    }

    // 카드 비밀번호 앞 2자리 입력
    while (true) {
        cout << "카드 비밀번호 앞 2자리: ";
        cin >> pwdFront;
        if (pwdFront == "exit") { cout << "회원가입 취소\n"; return; }
        if (pwdFront.length() != 2) {
            cout << "[오류] 비밀번호를 정확하게 입력하세요.\n";
            continue;
        }
        break;
    }

    // 카드 소유자 이름 입력
    while (true) {
        cout << "카드 소유자 이름: ";
        cin >> owner;
        if (owner == "exit") { cout << "회원가입 취소\n"; return; }
        if (owner.empty()) {
            cout << "[오류] 이름을 입력하세요.\n";
            continue;
        }
        break;
    }

    PaymentCard card{ cardNum, expiry, cvc, owner, pwdFront };
    User user{ name, phone, id, pwd, card };
    users[id] = user;
    cout << "회원가입 완료!\n";
}

void login() {
    string id, pwd;
    cout << "아이디: "; cin >> id;
    cout << "비밀번호: "; cin >> pwd;

    if (users.count(id) && users[id].pwd == pwd) {
        currentUser = &users[id];
        cout << currentUser->name << "님 로그인 성공!\n";
    }
    else {
        cout << "로그인 실패\n";
    }
}

void adminLogin() {
    string pwd;
    cout << "관리자 비밀번호 (// test 용이라 보입니다 admin을 입력해주세요..): "; cin >> pwd;
    if (pwd == "admin") {
        isAdmin = true;
        cout << "관리자 로그인 성공\n";
    }
    else {
        cout << "비밀번호 오류\n";
    }
}

void rentUmbrella() {
    if (!currentUser) return;
    if (currentUser->hasRented) {
        cout << "[오류] 이미 대여 중인 우산이 있습니다. 먼저 반납하세요.\n";
        return;
    }

    string lockerId;
    cout << "대여할 보관함 ID 입력 (A~E): "; cin >> lockerId;

    Umbrella* umb = findAvailableUmbrella(lockerId);
    if (!umb) {
        cout << "해당 보관함에 사용 가능한 우산 없음\n";
        return;
    }

    cout << "보관함 " << lockerId << "에서 우산 스캔됨: ID = " << umb->id << endl;
    cout << "대여 시작 버튼을 누르세요 (y 입력): ";
    char confirm;
    cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        umb->isAvailable = false;
        currentUser->rentUmbrella(*umb);
    }
    else {
        cout << "대여 취소됨\n";
    }
}

void returnUmbrella() {
    if (!currentUser) return;
    currentUser->returnUmbrella(lockers, 5);
}

void donateUmbrella() {
    if (!currentUser) return;
    int count, method;
    cout << "기부할 우산 수: "; cin >> count;
    cout << "1. 수거 2. 택배: "; cin >> method;
    donations.push_back({ count, method == 1 ? PICKUP : SHIPPING, currentUser->id });
    cout << "기부 신청 완료\n";
}

void reportIssue() {
    if (!currentUser || !currentUser->hasRented) {
        cout << "신고할 우산 없음\n";
        return;
    }
    int type;
    cout << "1. 분실 2. 파손: "; cin >> type;
    reports.push_back({ currentUser->rentedUmbrella.id, type == 1 ? LOST_REPORT : DAMAGED_REPORT, currentUser->id });
    cout << "신고 완료\n";
    currentUser->hasRented = false;
}

void showRewards() {
    if (!currentUser) return;
    cout << "새싹 수: " << currentUser->seeds << ", 지구 성장 단계: ";
    if (currentUser->seeds < 10) cout << "새싹";
    else if (currentUser->seeds < 20) cout << "나무";
    else cout << "지구";
    cout << endl;
}

void deleteAccount() {
    if (!currentUser) return;
    string pwd;
    cout << "회원 탈퇴를 진행합니다. 비밀번호를 입력하세요: ";
    cin >> pwd;
    if (pwd == currentUser->pwd) {
        users.erase(currentUser->id);
        cout << "회원 탈퇴 완료. 이용해 주셔서 감사합니다.\n";
        currentUser = nullptr;
    }
    else {
        cout << "비밀번호가 일치하지 않습니다. 탈퇴 취소.\n";
    }
}

void logout() {
    if (!currentUser) return;
    cout << currentUser->name << "님 로그아웃 되었습니다.\n";
    currentUser = nullptr;
}

void adminMenu() {
    int c;
    while (isAdmin) {
        cout << "\n[관리자 메뉴]\n1. 신고 목록\n2. 기부 목록\n3. 회원 목록\n4. 로그아웃\n> ";
        cin >> c;
        if (c == 1) {
            cout << "\n[신고 목록]\n";
            for (auto& r : reports)
                cout << "사용자ID: " << r.userId << ", 우산ID: " << r.umbrellaId
                << ", 유형: " << (r.type == LOST_REPORT ? "분실" : "파손") << endl;
        }
        else if (c == 2) {
            cout << "\n[기부 목록]\n";
            for (auto& d : donations)
                cout << "사용자ID: " << d.userId << ", 기부 수: " << d.count
                << ", 방식: " << (d.method == PICKUP ? "수거" : "택배") << endl;
        }
        else if (c == 3) {
            cout << "\n[회원 목록]\n";
            for (const auto& pair : users) {
                const User& u = pair.second;
                cout << "이름: " << u.name << ", ID: " << u.id
                    << ", 전화번호: " << u.phone << endl;
            }
        }
        else if (c == 4) {
            isAdmin = false;
            cout << "관리자 로그아웃\n";
        }
        else {
            cout << "잘못된 입력\n";
        }
    }
}

int main() {
    // 기본 우산들 초기화 (예시)
    for (char lid = 'A'; lid <= 'E'; ++lid) {
        Locker& locker = lockers[lid - 'A'];
        locker.id = string(1, lid);
        for (int i = 1; i <= 5; ++i) {
            Umbrella u{ to_string(lid) + to_string(i), NORMAL, true };
            locker.umbrellas.push_back(u);
        }
    }

    int choice;
    while (true) {
        if (currentUser) {
            cout << "\n--- 우산 대여 시스템 ---\n";
            cout << currentUser->name << "님 로그인 상태\n";
            cout << "1. 지도 보기\n";
            cout << "2. 우산 대여\n";
            cout << "3. 대여 상태 확인\n";
            cout << "4. 반납\n";
            cout << "5. 기부\n";
            cout << "6. 신고\n";
            cout << "7. 리워드\n";
            cout << "8. 회원 탈퇴\n";
            cout << "9. 로그아웃\n";
        
            cout << "0. 종료\n> ";
            cin >> choice;

            switch (choice) {
            case 1: showLockers(); break;
            case 2: rentUmbrella(); break;
            case 3: currentUser->showRentalStatus(); break;
            case 4: returnUmbrella(); break;
            case 5: donateUmbrella(); break;
            case 6: reportIssue(); break;
            case 7: showRewards(); break;
            case 8: deleteAccount(); break;
            case 9: logout(); break;
           
            case 0: cout << "프로그램 종료\n"; return 0;
            default: cout << "잘못된 입력\n"; break;
            }
        }
        else if (isAdmin) {
            adminMenu();
        }
        else {
            cout << "\n--- 우산 대여 시스템 ---\n";
            cout << "1. 회원가입\n";
            cout << "2. 로그인\n";
            cout << "3. 관리자 로그인\n";
            cout << "0. 종료\n> ";
            cin >> choice;

            switch (choice) {
            case 1: signup(); break;
            case 2: login(); break;
            case 3: adminLogin(); break;
            case 0: cout << "프로그램 종료\n"; return 0;
            default: cout << "잘못된 입력\n"; break;
            }
        }
    }
}
