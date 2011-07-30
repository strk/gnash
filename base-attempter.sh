#!/bin/sh
succeded=
failed=
if test x"$PKGCOMMAND" = x; then
  echo "Please execute deb-attempt-install-dependencies.sh or"
  echo "rpm-attempt-install-dependencies.sh after configuring."
  exit
fi

if test x"$packages" != x; then
  for PKGS in $packages; do
    for P in `echo $PKGS | sed "s/\\// /g"`; do
       $PKGCOMMAND $P
      success=$?
      if test $success -eq 0; then break; fi
    done
    if test $success -gt 0; then
      failed="$failed $PKGS"
    else
      succeded="$succeded $P"
    fi
  done

  echo
  echo "results:"
  for P in $succeded; do
    echo "SUCCESS: $P"
  done
  for P in $failed; do
    echo "FAILED: $P"
  done
  if test x"$failed" != x; then
    echo "Some packages might not have been installed because they are not in your repositories. This is normal. If needed, please install them manually. If packages exist, but were not detected, please tell us so we can add them to the list. For instructions on how to proceed from here, please run configure again."
  else
    echo "All dependencies are satisfied -- You should now be able to run configure successfully"
  fi
else
  echo "All dependencies are satisfied -- You should now be able to run configure successfully"
fi

