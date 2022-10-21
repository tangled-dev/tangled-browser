const getDateYear = () => {
  const myDate = new Date();
  const yearDate = myDate.getFullYear();
  const yearText = document.querySelectorAll(".copyrightTxt");
  yearText.forEach((item)=>{
    if(item){
      item.textContent = yearDate
    }
  })
 
};

getDateYear();
