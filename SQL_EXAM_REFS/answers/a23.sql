with 
countTable as (
	select Student_ID,COUNT(*) as No_Courses from Enrollment
	group by Student_ID
),
avgCourse as (
	select AVG(No_Courses) as avg_Courses from countTable
)


select COUNT(*) as answer from countTable
where No_Courses > (select avg_Courses from avgCourse);
