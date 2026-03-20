select S.Name from Student S
left join Enrollment E
on E.Student_ID=S.Student_ID
left join Course C
on C.Course_ID=E.Course_ID
left join Department D
on D.Department_ID=C.Department_ID
group by S.Student_ID
having COUNT(DISTINCT D.Department_ID) > 1;
