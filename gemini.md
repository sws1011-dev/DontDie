\# Project Context: DontDie



\## 1. 핵심 환경 정보

\- \*\*Engine:\*\* Unreal Engine 5.7.4 (최신 API 기준 사용)

\- \*\*Genre:\*\* Quarter-view Shooter (쿼터뷰 슈팅)

\- \*\*Project Name:\*\* DontDie



\## 2. 코딩 및 에디터 규칙 (중요)

\- \*\*UPROPERTY:\*\* 기본적으로 `EditAnywhere` 지정자를 사용하며, 카테고리는 별도로 지정하지 않음.

&#x20; - 예: `UPROPERTY(EditAnywhere) float MoveSpeed;`

\- \*\*Logging:\*\* 별도의 로그 카테고리를 정의하지 않았으므로, 모든 로그는 `LogTemp`를 사용함.

&#x20; - 예: `UE\_LOG(LogTemp, Warning, ...)`

\- \*\*Naming:\*\* PascalCase를 사용하며, 포인터 변수는 `TObjectPtr<T>`를 권장함.



\## 3. 구현된 시스템 (기준점)

\- \*\*Weapon System:\*\* 현재 캐릭터가 바라보는 방향(커서 위치 기반)으로 액터를 일직선으로 스폰하여 날려보내는 방식임.

\- \*\*Control:\*\* 쿼터뷰 시점이며, 마우스 커서 위치를 따라 캐릭터가 회전함.



\## 4. AI(Gemini) 협업 지침

1\. 새로운 기능을 제안할 때 기존의 `LogTemp`와 카테고리 없는 `UPROPERTY` 스타일을 유지할 것.

2\. 5.7.4 버전의 최신 기능을 활용하되, 복잡한 아키텍처보다는 \*\*'프로토타이핑을 위한 직관적인 코드'\*\*를 우선할 것.

3\. 무기 시스템 확장 시, 현재의 '액터 발사' 방식을 유지하면서 기능을 덧붙일 것.

