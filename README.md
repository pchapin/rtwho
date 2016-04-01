
RTWho
=====

This folder contains the RTWho project, a multiserver server monitoring tool by the VTC Computer
Club. See the HTML documents in the www folder for more information about how to build and use
RTWho.

Note that this project contains a nested subproject named 'netstream.' The netstream library is
a C++ library that layers IOStreams-like features onto network connections. This library is
intended to be independent of RTWho and is thus documented separately and had its own build
control files. It is stored with the RTWho project because, at the moment, RTWho is the only
application using netstream.

In addition, the Win32 C++ client uses the Xerces XML parser. This can be downloaded from the
Apache Project's web site. The build control files assume Xerces-C v3.0.0 is used and access the
Xerces headers and libraries via the XERCESROOT environment variable.
