! $TOG: pmconfig.cpp /main/6 1997/07/21 08:35:31 barstow $
! proxy manager config file
!
! Each line has the format:
!    <serviceName> managed <startCommand>
!        or
!    <serviceName> unmanaged <proxyAddress>
!
lbx managed LBXPROXY
!
! substitute site-specific info
!xfwp unmanaged firewall:4444
