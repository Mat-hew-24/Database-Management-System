SELECT S.Name, C.Course_Name FROM Student S 
JOIN Enrollment E 
ON E.Student_ID = S.Student_ID 
JOIN Course C 
ON C.Course_ID = E.Course_ID 
WHERE C.Course_Name 
IN ('Linear Algebra', 'Nuclear Physics') 
ORDER BY S.Name;
