// 기본 헤더 파일 포함
#include <iostream>      // 입출력 관련
#include <vector>        // 동적 배열 사용
#include <string>        // 문자열 처리
#include <unordered_map> // 해시맵 구조 사용
#include <ctime>         // 시간 처리용
#include <algorithm>     // 정렬 등 알고리즘 함수
#include <iomanip>       // 입출력 포맷 설정
#include <sstream>       // 문자열 스트림

using namespace std;

// [Enum 정의] 우산 상태 정의됨
enum UmbrellaStatus { NORMAL, LOST, DAMAGED }; // 정상, 분실, 파손
enum ReportType { LOST_REPORT, DAMAGED_REPORT }; // 신고 종류
enum DonationMethod { PICKUP, SHIPPING }; // 기부 방식
// [결제 카드 정보] 사용자 카드 등록용 구조체 선언됨
struct PaymentCard {
    string cardNumber;
    string expiryDate;
    string cvc;
    string ownerName;
    string pwdFrontTwoDigits;
};

// [우산 정보 구조체] 우산 ID, 상태, 대여 가능 여부 저장됨
struct Umbrella {
    string id;
    UmbrellaStatus status;
    bool isAvailable;
};

// [보관함 구조체] 보관함 ID와 우산 목록을 가짐
struct Locker {
    string id;
    vector<Umbrella> umbrellas;
};

// [신고 구조체] 우산 ID, 신고 유형, 사용자 ID 저장됨
struct Report {
    string umbrellaId;
    ReportType type;
    string userId;
};

// [기부 구조체] 기부 수량, 방법, 사용자 ID 저장됨
struct Donation {
    int count;
    DonationMethod method;
    string userId;
};

// [사용자 클래스] 사용자 정보와 관련 기능 포함됨
class User {
public:
    // 사용자 정보
    string name, phone, id, pwd;
    PaymentCard card; // 등록된 카드 정보
    Umbrella rentedUmbrella; // 대여 중인 우산
    time_t rentTime = 0; // 대여 시작 시간
    int seeds = 0; // 리워드 점수
    bool hasRented = false; // 대여 여부

    // 우산 대여 처리 함수
    // 전달받은 우산을 대여 상태로 설정하고, 대여 시작 시간을 기록함
    void rentUmbrella(Umbrella umbrella) {
        rentedUmbrella = umbrella;    // 대여한 우산 정보 저장
        rentTime = time(0);           // 현재 시간(대여 시작 시간) 기록
        hasRented = true;             // 대여 중 상태 표시

        // 대여 시작 시간 문자열로 변환 (예: "Thu Jun 19 12:34:56 2025\n")
        char buffer[26];
        ctime_s(buffer, sizeof(buffer), &rentTime);

        // 대여 시작 메시지 출력
        cout << "· 대여 시작됨! 우산 ID: " << umbrella.id << " | 시간 측정 시작: " << buffer;
    }


    // 우산 반납 처리 함수
    // 우산을 대여 중인지 확인하고, 대여 시간이 1시간을 초과했으면 비용을 계산해 결제 시도
    // 결제 성공 시 반납 완료, 실패 시 오류 메시지 출력 후 함수 종료
    void returnUmbrella(Locker lockers[], int lockerCount) {
        if (!hasRented) {
            // 대여 중인 우산이 없으면 메시지 출력 후 종료
            cout << "· 대여 중인 우산이 없습니다.\n";
            return;
        }

        time_t now = time(0);
        int seconds = difftime(now, rentTime);  // 대여 후 경과 시간(초) 계산

        if (seconds > 3600) {  // 1시간(3600초) 초과 시
            int hours = seconds / 3600;          // 초과한 시간 계산
            int cost = hours * 1000;             // 시간당 1000원 비용 산정
            cout << "· 1시간 초과: 자동 결제 시도 중... (" << cost << "원)\n";

            if (processPayment(cost)) {           // 결제 시도
                cout << "· 결제 성공\n";
            }
            else {
                // 결제 실패 시 오류 메시지 출력 후 반납 처리 중단
                cout << "[오류] 결제 실패! 카드 정보를 다시 확인해주세요.\n";
                return;
            }
        }
        else {
            // 1시간 이내 반납 시 별도 결제 없이 완료 메시지 출력
            cout << "· 우산 반납 완료 (1시간 이내)\n";
        }

        // 반납할 보관함 입력받고 해당 위치에 우산 등록됨
       // + 하드웨어 완벽 구현하면 자동으로 우산 id가 태그정보를 통해 보관함에 들어옴
        string lockerId;
        cout << "반납할 보관함 ID 입력 (A~E): ";
        cin >> lockerId;

        // 보관함 배열에서 ID 일치하는 곳에 반납 처리

        for (int i = 0; i < lockerCount; ++i) {
            if (lockers[i].id == lockerId) {
                rentedUmbrella.isAvailable = true;
                lockers[i].umbrellas.push_back(rentedUmbrella);
                break;
            }
        }

        hasRented = false; // 사용자 대여 상태 -> 해제
        rentTime = 0;  //대여시간 초기화
        seeds++; //리워드 점수 증가
    }
    // 결제 정보 유효성 검사 후 처리됨
    bool processPayment(int amount) {
        // 카드 정보 조건 만족 시 결제 성공으로 처리됨

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

unordered_map<string, User> users; // 사용자 목록
vector<Report> reports; // 신고 목룍
vector<Donation> donations; // 기부 목록
Locker lockers[5] = { {"A"}, {"B"}, {"C"}, {"D"}, {"E"} }; //보관함
User* currentUser = nullptr; // 현재 로그인한 사용자 
bool isAdmin = false; // 관리자 로그인 여부

// 지정된 보관함 ID에서 사용 가능한 정상 우산을 찾아 반환하는 함수
// 우산 목록을 ID 기준으로 정렬한 뒤, 상태가 NORMAL이고 사용 가능한 첫 번째 우산을 포인터로 반환
// 사용 가능한 우산이 없으면 nullptr 반환
Umbrella* findAvailableUmbrella(string lockerId) {
    for (auto& locker : lockers) {
        if (locker.id == lockerId) {
            // 우산 목록을 우산 ID 기준으로 오름차순 정렬
            sort(locker.umbrellas.begin(), locker.umbrellas.end(), [](const Umbrella& a, const Umbrella& b) {
                return a.id < b.id;
                });
            // 정상 상태이고 사용 가능한 우산을 찾음
            for (auto& umb : locker.umbrellas) {
                if (umb.status == NORMAL && umb.isAvailable)
                    return &umb;  // 첫 번째 사용 가능한 우산 반환
            }
        }
    }
    // 해당 보관함에 사용 가능한 우산이 없으면 nullptr 반환
    return nullptr;
}

// 보관함별 사용 가능한 정상 우산 수를 출력하는 함수
void showLockers() {
    for (auto& locker : lockers) {
        int count = 0;
        // 해당 보관함 내 우산들을 순회하며 정상 상태이고 사용 가능한 우산 개수 세기
        for (auto& u : locker.umbrellas)
            if (u.status == NORMAL && u.isAvailable) count++;
        // 보관함 ID와 사용 가능한 우산 수 출력
        cout << locker.id << "보관함 - 우산 수: " << count << endl;
    }
}

// 카드 만료일 유효성 검사됨
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
// 회원가입 로직 실행됨
void signup() {
    string name, phone, id, pwd, cardNum, expiry, cvc, pwdFront, owner;


    cout << "회원가입 시작 (취소하려면 exit 입력) \n";
    // 사용자가 회원가입을 취소하면 이전 화면으로 돌아가도록 설정

// 사용자 정보 입력 순차적으로 받음
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
    if (users.count(id)) { // 중복 id 검사
        cout << "이미 존재하는 아이디입니다.\n";
        return;
    }
    cout << "비밀번호 : ";
    cin >> pwd;
    if (pwd == "exit") {
        cout << "회원가입 취소\n";
        return;
    }

    // 카드 정보 입력 반복 (유효성 검사 포함) -> 자동결제 기능이 꼭 필요하기 때문에 사용자가 제대로 카드 정보를 입력했는지 확인하는 과정이 필요함 
    // 오류 발생시 이어서 재입력 가능하도록 함
    // + 카드회사나 은행어플과 연동하여 결제시스템을 받아오도록 확장 가능
    cout << " [ 결제 정보 등록 ]\n";

    // 카드번호 입력
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
    // 카드/사용자 객체 생성 후 등록됨

    PaymentCard card{ cardNum, expiry, cvc, owner, pwdFront };
    User user{ name, phone, id, pwd, card };
    users[id] = user;
    cout << "회원가입 완료!\n";
}
// [로그인 함수] 사용자 아이디, 비밀번호를 입력받아 로그인 처리함

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
// [관리자 메뉴] 관리자 전용 메뉴로 신고 내역, 기부 내역 조회 및 처리 가능함

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

// 우산 대여 함수
void rentUmbrella() {
    if (!currentUser) return; // 로그인되지 않은 경우 종료
    if (currentUser->hasRented) { // 이미 대여한 우산이 있는 경우
        cout << "[오류] 이미 대여 중인 우산이 있습니다. 먼저 반납하세요.\n"; // 1인당 대여 가능 우산 1개 -> 그전에 빌린 우산 반납 안하면 대여 불가
        return;
    }

    string lockerId;
    cout << "대여할 보관함 ID 입력 (A~E): "; cin >> lockerId;

    // 사용 가능한 우산을 해당 보관함에서 찾음
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
        umb->isAvailable = false; // 우산을 대여했으므로 사용 불가능으로 표시
        currentUser->rentUmbrella(*umb); // 유저에게 우산 대여 처리
    }
    else {
        cout << "대여 취소됨\n";
    }
}

// 우산 반납 함수
void returnUmbrella() {
    if (!currentUser) return; // 로그인되지 않은 경우 종료
    currentUser->returnUmbrella(lockers, 5); // 유저가 보유한 우산을 보관함에 반납
}

// 우산 기부 함수
void donateUmbrella() {
    if (!currentUser) return; // 로그인되지 않은 경우 종료
    int count, method;
    cout << "기부할 우산 수: "; cin >> count;
    cout << "1. 수거 2. 택배: "; cin >> method; 
    // 기부 내역 저장
    donations.push_back({ count, method == 1 ? PICKUP : SHIPPING, currentUser->id });
    cout << "기부 신청 완료\n";
}

// 문제 신고 함수 (분실/파손)
void reportIssue() {
    if (!currentUser || !currentUser->hasRented) {
        cout << "신고할 우산 없음\n";
        return;
    }
    int type;
    cout << "1. 분실 2. 파손: "; cin >> type; // 신고 유형 선택
    // 신고 내역 저장
    reports.push_back({ currentUser->rentedUmbrella.id, type == 1 ? LOST_REPORT : DAMAGED_REPORT, currentUser->id });
    cout << "신고 완료\n";
    currentUser->hasRented = false; // 더 이상 우산을 대여 중이 아님 
}

// 보상(리워드) 확인 함수
void showRewards() {
    if (!currentUser) return; // 로그인되지 않은 경우 종료
    cout << "새싹 수: " << currentUser->seeds << ", 지구 성장 단계: ";
    // 새싹 수에 따라 성장 단계 출력
    if (currentUser->seeds < 10) cout << "새싹";
    else if (currentUser->seeds < 20) cout << "나무";
    else cout << "지구";
    cout << endl;
}

// [회원 탈퇴] 현재 로그인한 회원의 비밀번호를 확인한 후 회원 정보를 삭제하고 로그아웃 처리함
void deleteAccount() {
    if (!currentUser) return;  // 로그인된 사용자가 없으면 함수 종료

    string pwd;
    cout << "회원 탈퇴를 진행합니다. 비밀번호를 입력하세요: ";
    cin >> pwd;

    // 입력한 비밀번호가 현재 사용자 비밀번호와 일치하면 회원 탈퇴 처리
    if (pwd == currentUser->pwd) {
        users.erase(currentUser->id);  // 회원 목록에서 삭제
        cout << "회원 탈퇴 완료. 이용해 주셔서 감사합니다.\n";
        currentUser = nullptr;          // 로그아웃 처리
    }
    else {
        cout << "비밀번호가 일치하지 않습니다. 탈퇴 취소.\n";  // 비밀번호 불일치 시 탈퇴 취소
    }
}

// 현재 로그인한 사용자를 로그아웃 처리하는 함수
void logout() {
    if (!currentUser) return;  // 로그인된 사용자가 없으면 함수 종료
    cout << currentUser->name << "님 로그아웃 되었습니다.\n";
    currentUser = nullptr;     // 로그인 상태 해제
}

// 관리자 전용 메뉴 함수
void adminMenu() {
    int c;
    while (isAdmin) {  // 관리자 권한이 있을 때만 메뉴 실행
        cout << "\n[관리자 메뉴]\n1. 신고 목록\n2. 기부 목록\n3. 회원 목록\n4. 로그아웃\n> ";
        cin >> c;

        if (c == 1) {
            // 신고 목록 출력
            cout << "\n[신고 목록]\n";
            for (auto& r : reports)
                cout << "사용자ID: " << r.userId << ", 우산ID: " << r.umbrellaId
                << ", 유형: " << (r.type == LOST_REPORT ? "분실" : "파손") << endl;
        }
        else if (c == 2) {
            // 기부 목록 출력
            cout << "\n[기부 목록]\n";
            for (auto& d : donations)
                cout << "사용자ID: " << d.userId << ", 기부 수: " << d.count
                << ", 방식: " << (d.method == PICKUP ? "수거" : "택배") << endl;
        }
        else if (c == 3) {
            // 회원 목록 출력
            cout << "\n[회원 목록]\n";
            for (const auto& pair : users) {
                const User& u = pair.second;
                cout << "이름: " << u.name << ", ID: " << u.id
                    << ", 전화번호: " << u.phone << endl;
            }
        }
        else if (c == 4) {
            // 관리자 로그아웃 처리
            isAdmin = false;
            cout << "관리자 로그아웃\n";
        }
        else {
            cout << "잘못된 입력\n";  // 메뉴 번호가 올바르지 않을 때 출력
        }
    }
}


int main() {
    // 기본 우산들 초기화 (50으로 기본 설정) 
    for (char lid = 'A'; lid <= 'E'; ++lid) {
        Locker& locker = lockers[lid - 'A'];
        locker.id = string(1, lid);
        for (int i = 1; i <= 50; ++i) {
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
