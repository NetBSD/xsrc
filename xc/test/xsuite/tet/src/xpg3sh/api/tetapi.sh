# Copyright 1990 Open Software Foundation (OSF)
# Copyright 1990 Unix International (UI)
# Copyright 1990 X/Open Company Limited (X/Open)
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of OSF, UI or X/Open not be used in 
# advertising or publicity pertaining to distribution of the software 
# without specific, written prior permission.  OSF, UI and X/Open make 
# no representations about the suitability of this software for any purpose.  
# It is provided "as is" without express or implied warranty.
#
# OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
# EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
# PERFORMANCE OF THIS SOFTWARE.
#
# ***********************************************************************
#
# SCCS:		@(#)tetapi.sh	1.9 03/09/92
# NAME:		Shell API Support Routines
# PRODUCT:	TET (Test Environment Toolkit)
# AUTHOR:	Andrew Dingwall, UniSoft Ltd.
# DATE CREATED:	1 November 1990
#
# DESCRIPTION:
#	This file contains shell functions for use with the shell API.
#	It is sourced automatically by the shell TCM.
#	In addition it should be sourced by test purposes that are written as
#	separate shell scripts, by means of the shell . command.
#
#	The following functions are provided:
#
#		tet_setcontext
#		tet_setblock
#		tet_infoline
#		tet_result
#		tet_delete
#		tet_reason
#
# MODIFICATIONS:
#
#	Geoff Clare, 29 Jan 1992
#		Rewrite tet_setcontext() so context number will change.
#
# ***********************************************************************

#
# publicly available shell API functions
#

# set current context and reset block and sequence
# usage: tet_setcontext
# Note that the context cannot be set to $$ because when a subshell
# is started using "( ... )" the value of $$ does not change.
tet_setcontext(){
	# This sets context to a new, unused process ID without
	# generating a zombie process.
	TET_CONTEXT=`(:)& echo $!`
	TET_BLOCK=1
	TET_SEQUENCE=1
}

# increment the current block ID, reset the sequence number to 1
# usage: tet_setblock
tet_setblock(){
	TET_BLOCK=`expr ${TET_BLOCK:?} + 1`
	TET_SEQUENCE=1
}

# print an information line to the execution results file
# and increment the sequence number
# usage: tet_infoline args [...]
tet_infoline(){
	tet_output 520 "${TET_TPCOUNT:?} ${TET_CONTEXT:?} ${TET_BLOCK:?} ${TET_SEQUENCE:?}" "$*"
	TET_SEQUENCE=`expr $TET_SEQUENCE + 1`
}

# record a test result for later emmision to the execution results file
# by tet_tpend
# usage: tet_result result_name
# (note that a result name is expected, not a result code number)
tet_result(){
	TET_ARG1="${1:?}"
	if tet_getcode "$TET_ARG1"
	then
		: ok
	else
		tet_error "invalid result name \"$TET_ARG1\"" \
			"passed to tet_result"
		TET_ARG1=NORESULT
	fi

	echo $TET_ARG1 >> ${TET_TMPRES:?}
	unset TET_ARG1
}

# mark a test purpose as deleted
# usage: tet_delete test_name reason [...]
tet_delete(){
	TET_ARG1=${1:?}
	shift
	TET_ARG2N="$*"
	if test -z "$TET_ARG2N"
	then
		tet_undelete $TET_ARG1
		return
	fi

	if tet_reason $TET_ARG1 > /dev/null
	then
		tet_undelete $TET_ARG1
	fi

	echo "$TET_ARG1 $TET_ARG2N" >> ${TET_DELETES:?}
	unset TET_ARG1 TET_ARG2N
}

# print the reason why a test purpose has been deleted
# return 0 if the test purpose has been deleted, 1 otherwise
# usage: tet_reason test_name
tet_reason(){
	: ${1:}
	(
		while read TET_A TET_B
		do
			if test X"$TET_A" = X"$1"
			then
				echo "$TET_B"
				exit 0
			fi
		done
		exit 1
	) < ${TET_DELETES:?}

	return $?
}


# ******************************************************************

#
# "private" functions for internal use by the shell API
# these are not published interfaces and may go away one day
#


# tet_getcode
# look up a result code name in the result code definition file
# return 0 if successful with the result number in TET_RESNUM and TET_ABORT
# set to YES or NO
# otherwise return 1 if the code could not be found
tet_getcode(){
	TET_ABORT=NO
	TET_RESNUM=-1
	: ${TET_CODE:?}

	TET_A="${1:?}"
	eval "`sed '/^#/d; /^[ 	]*$/d' $TET_CODE | while read TET_B
	do
		eval set -- $TET_B
		if test X\"$2\" = X\"$TET_A\"
		then
			echo TET_RESNUM=\\"$1\\"
			echo TET_ABACTION=\\"$3\\"
			exit
		fi
	done`"
	unset TET_A

	case "$TET_RESNUM" in
	-1)
		unset TET_ABACTION
		return 1
		;;
	esac

	case "$TET_ABACTION" in
	""|Continue)
		TET_ABORT=NO
		;;
	Abort)
		TET_ABORT=YES
		;;
	*)
		tet_error "invalid action field \"$TET_ABACTION\" in file" \
			$TET_CODE
		TET_ABORT=NO
		;;
	esac

	unset TET_ABACTION
	return 0
}

# tet_undelete - undelete a test purpose
tet_undelete(){
	echo "g/^${1:?} /d
w
q" | ed - ${TET_DELETES:?}
}

# tet_error - print an error message to stderr and on TCM Message line
tet_error(){
	echo "$TET_PNAME: $*" 1>&2
	echo "510|${TET_ACTIVITY:-0}|$*" >> ${TET_RESFILE:?}
}

# tet_output - print a line to the execution results file
tet_output(){
	> ${TET_STDERR:?}
	awk 'END {
		if (length(tet_arg2) > 0)
			tet_sp = " ";
		else
			tet_sp = "";
		line = sprintf("%d|%s%s%s|%s", tet_arg1, tet_activity, \
			tet_sp, tet_arg2, tet_arg3);

		# ensure no newline characters in data
		nl = sprintf("\n");
		n = split(line, a, nl);
		if (n > 1)
		{
			line = a[1];
			for (i = 2; i <= n; i++)
			{
				if (a[i] != "")
					line = line " " a[i];
			}
		}

		# journal lines must not exceed 512 bytes
		if (length(line) > 511)
		{
			printf("warning: results file line truncated: prefix: %d|%s%s%s|\n", tet_arg1, tet_activity, tet_sp, tet_arg2) >tet_stderr;
			line = substr(line, 1, 511);
		}

		# line is now OK to print
		print line;
	}' "tet_arg1=${1:?}" "tet_arg2=$2" "tet_arg3=$3" \
		"tet_activity=${TET_ACTIVITY:-0}" \
		"tet_stderr=$TET_STDERR" /dev/null >> ${TET_RESFILE:?}

	if test -s $TET_STDERR
	then
		tet_error "`cat $TET_STDERR`"
	fi
	> $TET_STDERR
}

