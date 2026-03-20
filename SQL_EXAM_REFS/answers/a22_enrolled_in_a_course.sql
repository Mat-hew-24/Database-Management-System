select S.Name from Student S 
left join Enrollment E 
on E.Student_ID=S.Student_ID 
left join Course C 
on C.Course_ID=E.Course_ID 
where C.Course_Name = ( 
    select C1.Course_Name from Course C1 
    left join Department D1 
    on D1.Department_ID=C1.Department_ID 
    left join Enrollment E1
    on E1.Course_ID=C1.Course_ID
    group by C1.Course_ID 
    having AVG(E1.Grade) = (
        select MIN(avgGrade) from (
            select AVG(Grade) as avgGrade from Enrollment 
            group by Course_ID
        ) AS SUB
    )
);
