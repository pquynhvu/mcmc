set.seed(12345)

library(combinat)
library(purrr)

N = 20    # Number of cities
p = 5     # Number of disconnected pairs
m = 20000 # Number of circuits needed to break in equilibrium
n = 20000 # Number of circuits taken into the calculation of average time spent.
M = n + m # Total number of runs
lamda = 1 # Exponential parameter
x = 0; y = 0; z = 0; 

cities <- c(1:N)
pairs <- dim(combn(N, 2))[2] # Number of pairs of cities
pairs
for(a in 1:p){
  x[a] = rdunif(1, 1, N); 
  y[a] = rdunif(1, 1, N)
  for(b in 1:(a-1)){
    if(isTRUE(x[a] != y[a] || y[a] != y[a - b] && x[a] != x[a - b])){a = a + 1}
    else{a = a + 0}
  }
}
disconnected.pairs <- data.frame(x, y) 
disconnected.pairs #Random disconnected pairs

unif.matrix <- matrix(runif(N*N), N)         # Initiate U(0,1) random variables
distance = - (1/lamda)*log(unif.matrix)      # Inverse method - Sampling from Exp(1)
ind <- lower.tri(distance)
distance[ind] <- t(distance)[ind]            # d(i,j) = d(j,i) 
hist(distance,freq=F,main="Exponential(1) from Uniform by Inverse method")
for(i in 1:N){
  for(j in 1:N){
    for(k in 1:p){
      if(isTRUE(i == j)){distance[i, j] = 0} # d(i,i) = 0
      else if(isTRUE(distance[i, j] == distance[disconnected.pairs$x[k], disconnected.pairs$y[k]])){distance[i, j] = 0} #distance of disconnected pairs is 0
      else if(isTRUE(distance[i, j] == distance[disconnected.pairs$y[k], disconnected.pairs$x[k]])){distance[i, j] = 0}
      else{distance[i, j] = distance[i, j]}
    }
  }
}

DistanceIsValid <- function(lst, distance){
  len = length(lst);
  if (len  <2) return(FALSE)
  if (isTRUE(distance[lst[[1]], lst[[len]]]<=0)){
    return(FALSE)
  }
  for(idx in 2:len){
    if (isTRUE(distance[lst[[idx-1]], lst[[idx]]]<=0)){
      return(FALSE)
    }
  }
  return(TRUE)
}
GetTotalDistance <- function(list, distance){
  
  len = length(list)
  if (len < 2) return (0)
  totalDistance = totalDistance + distance[list[1], list[len]]
  for(idx in 2:len){
    totalDistance = totalDistance + distance[list[idx-1], list[idx]]
  }
  return(totalDistance)
}

# Generate first circuit

FindValidRoute <- function(N, distance){
  saveNodes = NULL; routeNodes = NULL;
  while(isTRUE(1==1)){
    #cat("count:");  print(count);
    saveNodeCount = 0;  routeNodeCount = 0;
    currentNode <- rdunif(1, 1, N)
    routeNodeCount = routeNodeCount + 1; 
    routeNodes[routeNodeCount] = currentNode
    saveNodeCount = saveNodeCount + 1; 
    saveNodes[currentNode] <- currentNode; 
    #cat("saveNodes:");  print(saveNodes);
    #cat("routeNodes:");  print(routeNodes);
    while(routeNodeCount < N && saveNodeCount < N){
      proposalNode <- rdunif(1, 1, N)
      if(is.na(saveNodes[proposalNode])==FALSE) next
      saveNodeCount = saveNodeCount + 1; saveNodes[proposalNode] <- proposalNode
      if(isTRUE(distance[currentNode, proposalNode] <= 0)) next
      routeNodeCount = routeNodeCount + 1;  
      routeNodes[routeNodeCount] <- proposalNode
      currentNode <- proposalNode
      #cat("saveNodes:");  print(saveNodes);
      #cat("routeNodes:");  print(routeNodes);
    }
    # print(routeNodes)
    if(isTRUE(routeNodeCount==N) && isTRUE(distance[routeNodes[1], routeNodes[N]] > 0)){
      cat("Valid route: ");print(routeNodes)
      return(routeNodes)
    }
    else{
      cat("Invalid route: ");print(routeNodes)
    }
    routeNodes = NULL
    saveNodes = NULL
  }
}
SwapRoute <- function(routes, interchangePos1, interchangePos2){
  newRoutes = routes;
  newRoutes[interchangePos1] = routes[interchangePos2]
  newRoutes[interchangePos2] = routes[interchangePos1]
  return(newRoutes)
}
CheckDuplicate <- function(saveList, checkList){
  
  if (is.null(saveList))  return(FALSE)
  
  for(idx1 in 1:length(saveList)){
    subList =  saveList[[idx1]]
    
    count = 0;
    len = length(subList)
    for(idx2 in 1:len){
      if (is.null(subList[[idx2]])) next
      if (is.null(checkList[idx2])) next
      if ( subList[[idx2]]!=checkList[[idx2]]) break;
      count = count + 1
    }
    if (count==len)  return(TRUE)
  }
  return(FALSE)
}
AddToList <- function(saveList,checkList){
  
  len = length(saveList) + 1
  saveList[[len]] <- checkList
  return(saveList)
}

curProposalState = FindValidRoute(N, distance)
totalDistanceM = 0; totalDistanceN = 0;
totalDuplication = 0
saveDuplication = NULL

for(runIdx in 1:(m+n)){
  #for(runIdx in 1:1){
  # print(runIdx)
  # Ensure interchangePos1 is less than interchangePos2
  interchangePos1 = rdunif(1, 1, N)
  interchangePos2 = rdunif(1, 1, N)
  if (interchangePos1>interchangePos2){
    tmp = interchangePos2
    interchangePos2 =  interchangePos1
    interchangePos1 =  tmp
  }
  #cat("interchangePos ", interchangePos1, interchangePos2); print("")
  
  # Swap route
  newProposalState = SwapRoute(curProposalState,interchangePos1,interchangePos2);
  list1 = NULL
  if (interchangePos1==1) list1 = list(newProposalState[1],newProposalState[2])
  else list1 = list(newProposalState[interchangePos1-1], newProposalState[interchangePos1], newProposalState[interchangePos1+1])
  if (DistanceIsValid(list1, distance)==FALSE) next
  
  list2 = NULL
  if (interchangePos2==N) list2 = list(newProposalState[N-1],newProposalState[N])
  else list2 = list(newProposalState[interchangePos2-1], newProposalState[interchangePos2], newProposalState[interchangePos2+1])
  if (DistanceIsValid(list2, distance)==FALSE) next
  
  list3 = list(newProposalState[1], newProposalState[N])
  if (DistanceIsValid(list3, distance)==FALSE) next
  
  # Check duplication
  fDuplication = FALSE
  if (runIdx>m){
    fDuplication = CheckDuplicate(saveDuplication, newProposalState)
    if (fDuplication){
      totalDuplication = totalDuplication + 1
    }
    else {
      saveDuplication = AddToList(saveDuplication, newProposalState)
    }
  }
  totalDistance = 0;
  if (isFALSE(fDuplication)){
    totalDistance = GetTotalDistance(newProposalState, distance)
  }
  
  if (runIdx>m) {
    totalDistanceN = totalDistanceN + totalDistance
  }
  else{
    totalDistanceM = totalDistanceM + totalDistance
  }
  
  # cat("newProposalState"); print(newProposalState)
  curProposalState = newProposalState
}

avgDistanceM = totalDistanceM/m

if (n>totalDuplication){
  avgDistanceN = totalDistanceN/(n-totalDuplication)
} else { avgDistanceN = GetTotalDistance(curProposalState, distance)
}

cat("Total duplication M"); print(totalDuplication)
cat("Total distance M"); print(totalDistanceM)
cat("Total distance N"); print(totalDistanceN)
cat("Average distance M"); print(avgDistanceM)
cat("Average distance N"); print(avgDistanceN)

