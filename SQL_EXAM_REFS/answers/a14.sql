select D.Department_Name as Dept_Name,AVG(E.Grade) as Grade 
from Enrollment E 
left join Course C 
on C.Course_ID=E.Course_ID 
left join Department D
on D.Department_ID=C.Department_ID 
group by D.Department_ID;
