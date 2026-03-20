select D.Department_Name,COUNT(*) as no_of_students 
from Department D 
left join Course C 
on C.Department_ID=D.Department_ID 
left join Enrollment E 
on E.Course_ID=C.Course_ID 
group by D.Department_ID;
