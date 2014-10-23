#!/bin/bash

HOST_OS=`uname`;

executable="../build.$HOST_OS/tp";

if [ $SKIP_TESTS ]; then
    exit 0;
fi

if [ $UNIT_TESTS ]; then
    $executable;
    rc=$?;
    exit $rc;
fi

/sbin/ifconfig

list="closure_test.js while_test.js func_test.js exp_test.js object_test.js string_test.js prototype_test.js member_test.js for_test.js array_test.js fp_test.js self_ref.js eval_test.js func_constructor_test.js throw_test.js switch_test.js properties_test.js typed_array.js func_bind_test.js func_apply_test.js timer_test.js file_test.js arguments_test.js module_test.js netif_test.js misc_test.js emit_test.js serial_test.js";

for l in $list; do 
	echo "============================"
        # inject dummy data for input requiring tests (e.g. serial test)
	echo "dummy data" | $executable $l;
	if [[ $? != 0 ]]; then
		echo "Test $l failed";
		exit 1;
	fi
	echo "Test $l completed successfully";
done
