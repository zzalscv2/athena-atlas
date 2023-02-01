<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <link rel="stylesheet" href="trf_stylesheet.css">
    <title>Plot comparison</title>
  </head>

<h1>Plot comparison</h1>
<main>
<ul id="galerie">

<?php
     $files = glob("images/*.*");
     for ($i=0; $i<count($files); $i++)
      {
        $image = $files[$i];
        $supported_file = array(
                'gif',
                'jpg',
                'jpeg',
                'png'
         );

         $ext = strtolower(pathinfo($image, PATHINFO_EXTENSION));
         if (in_array($ext, $supported_file)) {
            echo "<li>";
            echo '<button tabindex="1"><img src="'.$image .'" alt="bar chart">"';
             echo "</button></li>";

            } else {
                continue;
            }
          }
       ?>



	</ul>
</main>

</html>
