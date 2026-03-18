select C.Course_Name,AVG(E.Grade) 
from Enrollment E 
join Course C 
on C.Course_ID=E.Course_ID 
group by C.Course_ID;
