*** Comments ***
SIL tests: the real firmware binary (led_seq.elf, the same file you flash)
runs inside the Renode emulator. Robot Framework drives it.

Run:  renode-test tests/sil/led_seq.robot --variable ELF:${PWD}/build-fw/led_seq.elf

Times are VIRTUAL time inside the emulator, not wall-clock time.
Test IDs (SIL-xx) map to docs/TEST_PLAN.md.

*** Variables ***
${ELF}              ${CURDIR}/../../build-fw/led_seq.elf
${PLATFORM}         ${CURDIR}/nucleo_f446re.repl
${UART}             sysbus.usart2
${BUTTON}           sysbus.gpioPortC.UserButton
${STEP_RE}          STEP (\\d) DIR ([+-]) PERIOD (\\d+) T (\\d+)

*** Keywords ***
Create Nucleo
    Execute Command             mach create "nucleo"
    Execute Command             machine LoadPlatformDescription @${PLATFORM}
    Execute Command             sysbus LoadELF @${ELF}
    Create Terminal Tester      ${UART}    defaultPauseEmulation=true
    FOR    ${i}    IN RANGE    7
        ${id}=    Create LED Tester    sysbus.gpioPortC.led${i}    defaultTimeout=0
        Set Suite Variable    ${LED${i}}    ${id}
    END

Boot
    Create Nucleo
    Start Emulation
    Wait For Line On Uart       BOOT led_seq    timeout=1

Wait For Step
    [Arguments]    ${index}    ${timeout}=1.1
    ${m}=    Wait For Line On Uart    STEP ${index} DIR    timeout=${timeout}
    RETURN    ${m}

Only LED Should Be On
    [Documentation]    LED ${index} on, all 6 others off.
    [Arguments]    ${index}
    FOR    ${i}    IN RANGE    7
        IF    ${i} == ${index}
            Assert LED State    true     testerId=${LED${i}}
        ELSE
            Assert LED State    false    testerId=${LED${i}}
        END
    END

Step Time
    [Arguments]    ${line}
    ${m}=    Evaluate    re.search(r"T (\\d+)", """${line}""")    modules=re
    ${t}=    Evaluate    int($m.group(1))
    RETURN    ${t}

*** Test Cases ***
SIL-01 Boots And Prints Version
    Boot
    Wait For Line On Uart       STEP 0 DIR + PERIOD 1000 T    timeout=0.1
    Only LED Should Be On       0

SIL-02 Runs PC0 To PC6 And Wraps
    Boot
    FOR    ${i}    IN    1    2    3    4    5    6    0
        Wait For Step           ${i}
        Only LED Should Be On   ${i}
    END

SIL-03 Step Period Is 1000 ms
    Boot
    ${a}=    Wait For Step    1
    ${b}=    Wait For Step    2
    ${t1}=   Step Time    ${a.line}
    ${t2}=   Step Time    ${b.line}
    ${d}=    Evaluate    ${t2} - ${t1}
    Should Be True    ${d} >= 999 and ${d} <= 1001    period was ${d} ms

SIL-04 UART r Reverses Direction
    Boot
    Write Char On Uart          r
    Wait For Line On Uart       OK 0 DIR -    timeout=0.1
    Wait For Step               6
    Only LED Should Be On       6
    Wait For Step               5

SIL-05 UART f And s Change Speed With Limits
    Boot
    Write Char On Uart          f
    Wait For Line On Uart       OK 0 DIR + PERIOD 500    timeout=0.1
    FOR    ${i}    IN RANGE    5
        Write Char On Uart      f
    END
    Write Char On Uart          ?
    Wait For Line On Uart       OK 0 DIR + PERIOD 125 T    timeout=0.1
    FOR    ${i}    IN RANGE    6
        Write Char On Uart      s
    END
    Write Char On Uart          ?
    Wait For Line On Uart       PERIOD 2000 T    timeout=0.1

SIL-06 UART p Pauses And Resumes
    Boot
    Write Char On Uart          p
    Wait For Line On Uart       PAUSED    timeout=0.1
    Should Not Be On Uart       STEP 1    timeout=3
    Only LED Should Be On       0
    Write Char On Uart          p
    Wait For Step               1

SIL-07 Unknown Command Gives ERR
    Boot
    Write Char On Uart          x
    Wait For Line On Uart       ERR    timeout=0.1

SIL-08 Button Reverses Direction
    Boot
    Execute Command             ${BUTTON} Press
    Wait For Line On Uart       BTN 0 DIR -    timeout=0.2
    Execute Command             ${BUTTON} Release
    Wait For Step               6

SIL-09 Heartbeat LD2 Toggles Every Step
    Boot
    ${ld2}=    Create LED Tester    sysbus.gpioPortA.LD2    defaultTimeout=0
    Assert LED State            false    testerId=${ld2}
    Wait For Step               1
    Assert LED State            true     testerId=${ld2}
    Wait For Step               2
    Assert LED State            false    testerId=${ld2}