# DontDie — 쿼터뷰 좀비 아포칼립스 슈팅 액션

▶︎ <a href="https://drive.google.com/file/d/1-GNrCtK7QhOcku7y7eoWVm8KVkxpg1jr/view?usp=drive_link" target="_blank">데모 영상 보기</a> | ▶︎ <a href="https://docs.google.com/document/d/1ZOtI66oj7ezhdIimEW_edaD1ylrY63SRQjwtXn8Xl9c/edit?usp=sharing" target="_blank">상세 보고서 보기</a> | ▶︎ <a href="https://github.com/sws1011-dev/DontDie/releases" target="_blank">실행 파일</a>

<img src="./Gif/DontDie.gif" width="600" alt="DontDie Gameplay">

## 한 줄 요약
**장르**: 쿼터뷰 슈팅 / **제작 기간**: 2주 / **사용 기술**: C++ 20, Unreal Engine 5.7.4 / **1인 개발**

## 프로젝트 설계 및 관리 방향
### 1. 확장성을 고려한 아키텍처
플레이어, 무기, 발사체를 각각 독립된 액터로 분리 설계하여 결합도를 낮추는 느슨한 결합(Loose Coupling) 구조를 적용했습니다. 이를 통해 기능 확장 시 기존 코드를 보존하고 유지보수를 용이하게 만들었습니다.

### 2. 제한된 일정 내 자원 관리
2주라는 제약 조건에 맞춰 핵심 API와 제어 루프 중심으로 구현 범위를 명확히 한정했습니다. 마감 직전의 변수나 기획 변경 시에도 판을 뒤엎지 않고 기존 코드를 유연하게 재사용하여 일정을 준수했습니다.

### 3. 데이터 기반의 흐름 조율
추가 기능이나 데이터가 전체 시스템에 미치는 인과관계를 고려했습니다. 시스템 간의 정합성을 실시간으로 모니터링하며 오차 범위를 정밀하게 조율하여 안정적인 빌드를 도출했습니다.

## 핵심 기능 (3개로 압축)
1. **동적 조준 및 보정 시스템** — 마우스 2D 좌표를 3D 공간으로 역추적(Deproject)하고, Line Trace를 통해 지형 고저차에 따른 조준선 오차를 실시간으로 보정합니다.
2. **무기 모듈화 시스템 (Loose Coupling)** — 발사 로직을 캐릭터와 분리하여 독립적인 `AWeapon` 액터로 설계. WeaponSocket 결합 방식을 통해 무기 스왑 및 기능 확장이 용이한 구조를 구축했습니다.
3. **순환형 웨이브 제어 시스템** — 10웨이브 주기의 난이도 상승 공식과 `WaveTimerHandle`을 활용, 월드 내 잔여 적 수를 체크하여 끊김 없는 게임 루프를 자동 트리거합니다.

## 내가 직접 만든 부분
- **C++ 클래스 설계 및 구현 (100%)**: `APlayerPawn`, `ABullet`, `AWeapon`, `AEnemyFactory` 등 핵심 게임 로직 전체 구현
- **게임 시스템 로직**: 동적 조준, 발사 메커니즘, BoundingBox 기반 적 스폰, 웨이브/성장 시스템
- **사용 외부 에셋**: Mixamo (캐릭터/애니메이션), Korboleev (건물), Quixel Megascans (환경 텍스처 및 도로)

## 기술적 도전과 해결
- **문제**: 연사 속도 상향 시 효과음 출력 시점이 밀리며 오디오 싱크가 어긋나는 현상 발생 (수행 점검 및 철저 확인)
- **해결**: 데이터 흐름을 실시간으로 모니터링하여 병목 지점을 파악하고, 연사 배율에 대응하여 재생 피치(Pitch) 및 가속도를 동적으로 보정하는 로직을 적용해 청각적 몰입도를 확보했습니다.
- **배움**: 사후 검증 단계에서 오차가 없도록 결과물의 무결성을 스스로 모니터링하는 습관이 품질 향상의 핵심임을 깨달았습니다.

→ <a href="https://docs.google.com/document/d/1ZOtI66oj7ezhdIimEW_edaD1ylrY63SRQjwtXn8Xl9c/edit?usp=sharing" target="_blank">상세 트러블슈팅은 프로젝트 보고서 참조</a>

## 기술 스택
- **Language**: C++ 20
- **Engine**: Unreal Engine 5.7.4
- **IDE**: Visual Studio 2022 / Rider
- **VCS**: Git / GitHub

## 빌드 및 패키징 방법
### 1. 개발 환경 빌드 (C++ 소스 반영)
1. `DontDie.uproject` 파일을 우클릭하여 `Generate Visual Studio project files`를 수행합니다.
2. 생성된 `.sln` 파일을 열어 `Development Editor` 구성에서 빌드합니다.

### 3. 실행 파일(.exe) 생성 (패키징)
1. 언리얼 에디터를 실행합니다.
2. 상단 메뉴의 **Platforms > Windows > Package Project**를 선택합니다.
3. 결과물이 저장될 폴더를 지정하면 패키징이 시작됩니다.
4. 완료 후 지정한 폴더의 `Windows/DontDie.exe`를 통해 게임을 즉시 실행할 수 있습니다.

## 보고서·문서
- <a href="https://docs.google.com/document/d/1ZOtI66oj7ezhdIimEW_edaD1ylrY63SRQjwtXn8Xl9c/edit?usp=sharing" target="_blank">프로젝트 진행 보고서</a> — 기획부터 트러블슈팅까지 전 과정 기록
