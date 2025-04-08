
-- There is no need to set up an allowed arch that comes with the msvc

function windows_target()

    local use_llvm_toolchain = get_config("use-llvm")
    if use_llvm_toolchain then	
        set_toolchains("clang-cl")
    end

    if is_kind("binary") then
        set_extension(".exe")
    end

    if is_mode("debug") then
        add_cxflags("/guard:cf") -- enable Control Flow Guard
    end

    add_cxflags("-GR-") -- disable rtti

    local static_link = get_config("static")

    if is_mode("debug") then
        add_cxflags("-GS") -- enable buffer overflow check

        if static_link then	
            set_runtimes("MTd")
        else
            set_runtimes("MDd")
        end
    else
        add_cxflags("-GS-") -- disable buffer overflow check
        add_cxflags("-GL") -- Whole Program Optimization
        --add_ldflags("-LTCG") -- Link Time Code Generation

        if static_link then	
            set_runtimes("MT")
        else
            set_runtimes("MD")
        end
    end

    local opt_name = get_config("winmin")
    if opt_name == "default" then	
        -- same as WIN10
        add_defines("_WIN32_WINNT=0x0A00")
        add_defines("WINVER=0x0A00")
        add_ldflags("-SUBSYSTEM:CONSOLE")
        -- Do not set console version=10.00.
        -- This option may be changed in a future update of the NT kernel major version.

    -- Windows NT (with win32 api)

    elseif 
        opt_name == "WS25" or 
        opt_name == "WIN11" or 
        opt_name == "WS22" or
        opt_name == "WS19" or
        opt_name == "WS16" or
        opt_name == "WIN10"
            then
        add_defines("_WIN32_WINNT=0x0A00")
        add_defines("WINVER=0x0A00")
        add_ldflags("-SUBSYSTEM:CONSOLE")
        -- Do not set console version=10.00.
        -- This option may be changed in a future update of the NT kernel major version.
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS12R2" or
        opt_name == "WINBLUE"
            then
        add_defines("_WIN32_WINNT=0x0603")
        add_defines("WINVER=0x0603")
        add_ldflags("-SUBSYSTEM:CONSOLE,6.03")
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS12" or 
        opt_name == "WIN8" 
            then
        add_defines("_WIN32_WINNT=0x0602")
        add_defines("WINVER=0x0602")
        add_ldflags("-SUBSYSTEM:CONSOLE,6.02")
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS08R2" or
        opt_name == "WIN7"
            then
        add_defines("_WIN32_WINNT=0x0601")
        add_defines("WINVER=0x0601")
        add_ldflags("-SUBSYSTEM:CONSOLE,6.01")
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS08" or
        opt_name == "VISTA" 
            then
        add_defines("_WIN32_WINNT=0x0600")
        add_defines("WINVER=0x0600")
        add_ldflags("-SUBSYSTEM:CONSOLE,6.00")
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS03SP1" or
        opt_name == "WINXPSP2"
            then
        add_defines("_WIN32_WINNT=0x0502")
        add_defines("WINVER=0x0502")
        add_ldflags("-SUBSYSTEM:CONSOLE,5.02")
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS03" or
        opt_name == "WINXP"
            then
        add_defines("_WIN32_WINNT=0x0501")
        add_defines("WINVER=0x0501")
        add_ldflags("-SUBSYSTEM:CONSOLE,5.01")
        add_syslinks("ntdll")
    elseif 
        opt_name == "WS2K" or
        opt_name == "WIN2K"
    then
        add_defines("_WIN32_WINNT=0x0500")
        add_defines("WINVER=0x0500")
        add_ldflags("-SUBSYSTEM:CONSOLE,5.00")
        add_syslinks("ntdll")

    -- Windows 9x (with win32 api)，unverified.

    elseif opt_name == "WINME" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WIN32_WINDOWS=0x0490")
        add_defines("WINVER=0x0490")
        add_ldflags("-SUBSYSTEM:CONSOLE,4.90")
        add_syslinks("mscvrt")
    elseif opt_name == "WIN98" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WIN32_WINDOWS=0x0410")
        add_defines("WINVER=0x0410")
        add_ldflags("-SUBSYSTEM:CONSOLE,4.10")
        add_syslinks("mscvrt")
    elseif opt_name == "WIN95" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WIN32_WINDOWS=0x0400")
        add_defines("WINVER=0x0400")
        add_ldflags("-SUBSYSTEM:CONSOLE,4.00")
        add_syslinks("mscvrt")

    -- Windows NT (without win32 api)，unverified.

    elseif opt_name == "NT400" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WINNT=0x0400")
        add_defines("WINVER=0x0400")
        add_ldflags("-SUBSYSTEM:CONSOLE,4.00")
        add_syslinks("ntdll")
    elseif opt_name == "NT351" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WINNT=0x0351")
        add_defines("WINVER=0x0351")
        add_ldflags("-SUBSYSTEM:CONSOLE,3.51")
        add_syslinks("ntdll")
    elseif opt_name == "NT350" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WINNT=0x0350")
        add_defines("WINVER=0x0350")
        add_ldflags("-SUBSYSTEM:CONSOLE,3.50")
        add_syslinks("ntdll")
    elseif opt_name == "NT310" then -- unverified
        add_undefines("_WIN32_WINNT")
        add_defines("_WINNT=0x0310")
        add_defines("WINVER=0x0310")
        add_ldflags("-SUBSYSTEM:CONSOLE,3.10")
        add_syslinks("ntdll")
    else
        error("invalid value")
    end

    local march = get_config("march")
    if not (march == "no" or march == "default") then
        error("windows (unknown-windows-msvc) does not support custom march!")
    end
end