# Information Retrieval Term Project
### Korea Univ. 2016-2학기 COSE472

## 기간
    * 2016.09 ~ 2016.12

## 구현환경
    * Windows 10
    * Visual Studio 2015
    * C++ (Using STL, No extra library) 
    * Release x64

## 구현내용
    * Document Parsing
    * Stemming ( Using Porter2 Stemmer )
    * Stopword Removing (불용어 제거)
    * Indexing (역색인)
    * Ranking ( Vector Space Model & Language Model & Dirichelet Smoothing )
    * Evaluation ( 평가 )

## Recall-Precision Graph
<p align="center">
  <img src="./graph.jpg" >
</p>

## 실행방법
    * index 파일들은 이미 생성되어 있고, 아래의 링크에서 다운받아야합니다.
    * index 파일들의 압축을 풀고, 실행파일.exe와 같은 위치에 두고, exe 파일을 실행합니다.
    * 실행 결과 result.txt 결과 파일이 생성되고, evaluator.exe를 실행하면 결과파일 evaluation.txt가 출력됩니다.
    * 모두 같은 폴더에 있어야 정상적으로 작동합니다.

## 필요한 파일
    * corpus (기사 약 55만개) - https://drive.google.com/file/d/0B8VS6fJASyTvLXc0S2t0X2VWYm8/view?usp=sharing
    * Porter2 Stemmer - https://drive.google.com/file/d/0B8VS6fJASyTvVkItNUVvZ0RENGc/view?usp=sharing
