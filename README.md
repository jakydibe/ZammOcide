# Zammocide
A process killer that can kill protected processes abusing Zam64.sys driver (still not blacklisted by microsoft vulnerable driver blocklist)

We will exploit the unpatched CVE-2018-6606 to use a legitimate driver to kill any process we want.

Credits to:  https://rce4fun.blogspot.com/2018/02/malwarefox-antimalware-zam64sys.html for writing a post about CVE-2018-6606



# Usage

In order to use this just download the release or compile it from source and.

1) `./ZammOcide` without anything kills the 2 processes in my AV list (MsMpEng.exe and SecurityHealthService.exe)
2) `--pid <PID>` to specify a PID you do want to kill
3) `--loop` to keep the program running and killing the processes whenever they spawn
4) `--path <PATH>` to specify zam64.sys path.  
