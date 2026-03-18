select D.*,F.Name as Faculty_Head 
from Department D 
join Faculty F 
on F.Faculty_ID=D.Head_of_Dept_ID;
