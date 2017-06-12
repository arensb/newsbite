#!/bin/sh
testno=1

start_test() {
	echo "=== test $testno"
}

end_test() {
	echo "end $testno"
	testno=`expr $testno + 1`
}

start_test
echo "first"
end_test

start_test
echo "second"
end_test

start_test
echo "third"
end_test
