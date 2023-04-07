# SIC-XE-Assembler
Soongsil University. System Programming 2021-1 Project1A - Two pass Assembler for SIC/XE written in C
## 🧑‍💻: Intro
소스코드 (input.txt)를 입력 받아 label, operator, operand, comment로 구분하여 토큰화 해서 파싱하고,
inst.data 파일로부터 파싱한 instruction set을 기반으로 operand에 해당하는 opcode를 매핑 시킴으로써 기본적인 SIC / XE 머신의 작동 원리를 이해할 수 있습니다.
![1](https://user-images.githubusercontent.com/75844701/224520533-acb44d62-b3b1-4eb4-bf33-fb64456e1547.png)

![2](https://user-images.githubusercontent.com/75844701/224520854-9b684944-0e41-4327-922c-ba798182537a.png)

![3](https://user-images.githubusercontent.com/75844701/224520861-5cd224de-9444-48bf-9e2c-a708d80d577c.png)

![6](https://user-images.githubusercontent.com/75844701/224520548-f393c0ba-c5e1-41ea-a16e-27e6cb5dccdb.png)
</br>
</br>
inst.data :
Opcodes used in program.
</br>

input.txt :
SIC-XE source code that is parsed in orde to produce output.
</br>

token_parsing(char *str) :
Parse the parameter (input_data) into 'token_table' structure.
To distinguish character by character, add  'cmp' buffer.
As the word is set in 'cmp', its value is store on 'label', 'operator', 'operand', 'comment' sequentially.

</br>
</br>

## 📞: Contact

- 이메일: hyeonwoody@gmail.com
- 블로그: https://velog.io/@hyeonwoody
- 깃헙: https://github.com/hyeonwoody

</br>

## 🧱: Technologies Used
>C
