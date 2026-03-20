with fails as (
	select DISTINCT S.Student_ID from Student S
	left join Enrollment E
	on E.Student_ID=S.Student_ID
	where E.Grade < 70
),
rowfails as (
	select COUNT(*) as actual_fail_count from fails
),
stu_count as (
	select COUNT(*) as actual_stu_count from Student
)

select ROUND((actual_fail_count * 100.0 / actual_stu_count), 2) as Fail_Percentage
from rowfails, stu_count;
