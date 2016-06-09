This is a library for creating xll add-in's for versions of Excel before
and after Excel 2007.  It makes every feature of the Excel 2010 SDK
available to you, including 64-bit builds.

INSTALLATION

Run setup.msi and project.msi from http://xll.codeplex.com.

OPERATION

In Visual Studio 2010 create a new project of type XLL AddIn Project.
Right click on project, Build.
Right click on project, Debug > Start new instance.

Set breakpoints by clicking to the left of the line.

USAGE

The naming convention is the same as in the Microsoft SDK: just add a
'12' at the end of names.

If you add an 'X' instead of '12' you can use the same source
code to build add-ins for either version.  Just define EXCEL12
before including "xll/xll.h" to create add-ins for the latest
versions of Excel.  We use the same UNICODE conventions as
Microsoft. http://msdn.microsoft.com/en-us/goglobal/bb688113.aspx

TROUBLESHOOTING
http://support.microsoft.com/default.aspx?scid=kb;en-us;Q280504
http://support.microsoft.com/kb/948615

BUILDS

You can target multiple platforms when using VC++ 2012.
Set Properties>General>Platform Toolset.
Be sure to use same toolset for every project in the solution.
(Note you can select all projects with Ctrl-Left-Click.)

HELP FILES
 1. Run ‘regedit’ command from the command line.
 2. Locate and then click the following subkey:
 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\ItssRestrictions
 Note: If this registry subkey does not exist, then create it.
 3. Right-click the ItssRestrictions subkey, point to New, and then click DWORD Value.
 4. Type MaxAllowedZone, and then press ENTER.
 5. Right-click the MaxAllowedZone value, and then click Modify.
 6. In the Value data box, type a number from 0 and 4, and then click OK.
 The values settings are
 0 = My Computer
 1 = Local Intranet Zone
 2 = Trusted sites Zone
 3 = Internet Zone
 4 = Restricted Sites Zone