#!/usr/local/bin/perl

# This script drives the xauth program exercising different options
# and verifying that the generated authorizations behave in the expected
# way.  To use it, you should start the server with an authorization file
# specified with the -auth command line option, and you should start at
# least one client to prevent server resets.  The environment that this
# script runs in should have the XAUTHORITY environment variable set to
# an authority file that allows connections to the server.  It can
# be the same file that you pass to the server provided that the display
# stored in the auth file matches the DISPLAY.

$display  = $ENV{'DISPLAY'};    # server to contact
$errors = 0;			# no errors yet
$utclient = "xset q";	# a client that should work fine when run untrusted
$initauth = $ENV{'XAUTHORITY'}; # remember the initial authorization
if ($initauth eq '')
{
    $initauth = <~/.Xauthority>;
}

print "Using display ", $display, " with authority file ", $initauth, "\n";

# tests begin here

$m = "verify that we can connect with a generated authorization";

# Plan: generate an authorization and attempt a connection with it.
# If the connection does not succeed, the test fails.

`xauth -f a1 g $display .`;
$ENV{'XAUTHORITY'} = "a1";
`$utclient`;
if ($? != 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}

unlink "a1";


$m = "verify that a trusted authorization can do trusted operations";

# Plan: generate a trusted authorization and attempt to run xlsclients using
# it.  (xlsclients does privileged operations that untrusted clients can't
# do.)  If xlsclients does not succeed, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
`xauth -f a1 g $display . trusted`;
$ENV{'XAUTHORITY'} = "a1";
`xlsclients`;
if ($? != 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}
unlink "a1";


$m = "verify that an untrusted authorization cannot do trusted operations";

# Plan: generate an untrusted authorization and attempt to run xlsclients using
# it.  (xlsclients does privileged operations that untrusted clients can't
# do.)  If xlsclients succeeds, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
`xauth -f a1 g $display . untrusted`;
$ENV{'XAUTHORITY'} = "a1";
`xlsclients`;
if ($? == 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}
unlink "a1";


$m = "verify that an attempt to connect after the timeout fails";

# Plan: generate an authorization with a short timeout.  Wait for a period
# longer than the timeout, then and attempt a connection using that
# authorization.  If the connection succeeds, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
`xauth -f a1 g $display . timeout 1`;
sleep 3;
$ENV{'XAUTHORITY'} = "a1";
`$utclient`;
if ($? == 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}
unlink "a1";


$m = "add multiple auths and verify that we can connect with all of them";

# Plan: generate two authorizations, then and attempt a connections using
# both of them.  If either connection fails, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
`xauth -f a1 g $display .`;
`xauth -f a2 g $display .`;

$ENV{'XAUTHORITY'} = "a1";
`$utclient`;
if ($? != 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}

$ENV{'XAUTHORITY'} = "a2";
`$utclient`;
if ($? != 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}

unlink "a1", "a2";


$m = "verify that an attempt to create an auth from an untrusted client fails";

# Plan: generate an untrusted authorization, then use it to connect and
# attempt to generate a second authorization.  If the second creation
# succeeds, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
`xauth -f a1 g $display . untrusted`; # create untrusted auth
$ENV{'XAUTHORITY'} = "a1";
`xauth -f a2 g $display . trusted`;  # attempt to create auth using untrusted auth
if ($? == 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}

unlink "a1", "a2";


$m = "specify invalid authorization protocol name";

# Plan: specify an invalid authorization protocol name to xauth.
# If xauth succeeds, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
`xauth -f a1 g $display FOOBAR`;
if ($? == 0)
{
    print "ERROR: ", $m, "\n";
    $errors++;
}


$m = "stress test: create lots of authorizations";

# Plan: create a hundred authorizations using different timeouts.
# If any of the creates fail, the test fails.

$ENV{'XAUTHORITY'} = $initauth;
$i = 100;
while ($i--)
{
    `xauth -f a1 g $display . timeout $i`;
    if ($? != 0)
    {
	print "ERROR: ", $m, "\n";
	$errors++;
    }
}

print $errors, " errors\n";
