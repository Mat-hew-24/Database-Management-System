SELECT S.Name FROM Student S 
JOIN Enrollment E 
ON E.Student_ID = S.Student_ID 
GROUP BY S.Student_ID 
HAVING COUNT(*) >= 2;
