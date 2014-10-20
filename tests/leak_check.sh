#!/bin/bash

executable="../build.Linux/tp";

if [ $SKIP_TESTS ]; then
    exit 0;
fi

function lc()
{
    valgrind -v --leak-check=full --num-callers=50 --show-reachable=yes $executable $1 &> $1.lc;
}

if [ $UNIT_TESTS ]; then
    lc unit_test
    grep "ERROR SUMMARY" unit_test.lc;
    exit 0;
fi

list="closure_test.js while_test.js func_test.js exp_test.js object_test.js string_test.js prototype_test.js member_test.js for_test.js array_test.js fp_test.js self_ref.js eval_test.js func_constructor_test.js throw_test.js switch_test.js properties_test.js typed_array.js func_bind_test.js func_apply_test.js timer_test.js file_test.js arguments_test.js module_test.js emit_test.js";

if [[ -n $1 ]] ; then
	lc $1;
	exit;
fi
for l in $list; do 
	echo "============================"
	lc $l;
	grep "ERROR SUMMARY" $l.lc;
	if [[ $? != 0 ]]; then
		echo "Test $l failed";
		break;
	fi
	echo "Test $l completed successfully";
done

