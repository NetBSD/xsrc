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
