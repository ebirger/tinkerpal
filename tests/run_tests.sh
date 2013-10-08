#!/bin/bash

if [ $SKIP_TESTS ]; then
    exit 0;
fi

list="closure_test.js while_test.js func_test.js exp_test.js object_test.js string_test.js prototype_test.js member_test.js for_test.js array_test.js fp_test.js self_ref.js eval_test.js func_constructor_test.js throw_test.js switch_test.js properties_test.js typed_array.js";

HOST_OS=`uname`;

executable="../build.$HOST_OS/tp";

for l in $list; do 
	echo "============================"
	$executable $l;
	if [[ $? != 0 ]]; then
		echo "Test $l failed";
		exit 1;
	fi
	echo "Test $l completed successfully";
done

