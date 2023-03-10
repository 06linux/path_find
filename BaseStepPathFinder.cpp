
#include "BaseStepPathFinder.h"
static BaseStepPathFinder* gBaseStepPathFinder = NULL;

BaseStepPathFinder* BaseStepPathFinder::Instance()
{
    if(NULL == gBaseStepPathFinder)
    {
        gBaseStepPathFinder = new BaseStepPathFinder();
    }
    
    return gBaseStepPathFinder;
}
BaseStepPathFinder::BaseStepPathFinder()
{
    mapWidth = 0;
	mapHeight = 0;
	
	pMapData = NULL;
	pMapDataInit = NULL;
	
	currSearchHeight = 0;	
}
BaseStepPathFinder::~BaseStepPathFinder()
{
    FreeMapData();
}
void BaseStepPathFinder::InitMapData(int _mapWidth, int _mapHeight)
{
    if(_mapWidth <=0 || _mapHeight <= 0) return;
	
	FreeMapData();		// 先清除
	
	mapWidth = _mapWidth;
	mapHeight = _mapHeight;
	
	pMapData = MALLOCXX(mapHeight, mapWidth, int);
	pMapDataInit = MALLOCXX(mapHeight, mapWidth, int);
	
	for(int i=0; i<ARRLEN(pMapData); i++)   // 行
	{
		for(int j=0; j<ARRLEN(pMapData[i]); j++) // 列
		{
			pMapData[i][j] = BaseStepPathFinder_MaxHeight;		// 默认可以通过
			pMapDataInit[i][j] = BaseStepPathFinder_MaxHeight;	// 默认可以通过
		}
	}
}
void BaseStepPathFinder::FreeMapData()
{
    FREEXX(pMapData);
	FREEXX(pMapDataInit);
	
	mapWidth = 0;
	mapHeight = 0;
}
void BaseStepPathFinder::SetPass(BasePoint _mapGrid, bool _isCanPass)
{
    int mapGridX = _mapGrid.x;
	int mapGridY = _mapGrid.y;
	
	if(mapGridX < 0 || mapGridX >= mapWidth)	return;
	if(mapGridY < 0 || mapGridY >= mapHeight)	return;
	
	if(mapGridY < ARRLEN(pMapDataInit) && mapGridX < ARRLEN(pMapDataInit[mapGridY]) )  // 防止数组越界
	{
		if(_isCanPass)
		{
			pMapDataInit[mapGridY][mapGridX] = BaseStepPathFinder_MaxHeight;
		}
		else
		{
			pMapDataInit[mapGridY][mapGridX] = -1;	// 不可以通过
		}
	} 
}
void  BaseStepPathFinder::MapDataSetPass(BasePoint _mapGrid, bool _isCanPass)
{
    int mapGridX = _mapGrid.x;
	int mapGridY = _mapGrid.y;
	
	if(mapGridX < 0 || mapGridX >= mapWidth)	return;
	if(mapGridY < 0 || mapGridY >= mapHeight)	return;
	
	if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		if(_isCanPass)
		{
			pMapData[mapGridY][mapGridX] = BaseStepPathFinder_MaxHeight;
		}
		else
		{
			pMapData[mapGridY][mapGridX] = -1;	// 不可以通过
		}
	}
}

void BaseStepPathFinder::SetMapData(BasePoint _mapGrid, int _value)
{
    int mapGridX = _mapGrid.x;
	int mapGridY = _mapGrid.y;
	
	if(mapGridX < 0 || mapGridX >= mapWidth)	return;
	if(mapGridY < 0 || mapGridY >= mapHeight)	return;
	
	if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		pMapData[mapGridY][mapGridX] = _value;
	}
}
int BaseStepPathFinder::GetMapData(BasePoint _mapGrid)
{
    int mapGridX = _mapGrid.x;
	int mapGridY = _mapGrid.y;
	
	if(mapGridX < 0 || mapGridX >= mapWidth)	return (-1);
	if(mapGridY < 0 || mapGridY >= mapHeight)	return (-1);
	
	if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		return pMapData[mapGridY][mapGridX];
	}
	
	return (-1);
}
bool BaseStepPathFinder::IsPass(BasePoint _mapGrid)
{
    int mapGridX = _mapGrid.x;
	int mapGridY = _mapGrid.y;
	
	if(mapGridX < 0 || mapGridX >= mapWidth)	return false;
	if(mapGridY < 0 || mapGridY >= mapHeight)	return false;
	
	if(mapGridY < ARRLEN(pMapDataInit) && mapGridX < ARRLEN(pMapDataInit[mapGridY]) )  // 防止数组越界
	{
		if(pMapDataInit[mapGridY][mapGridX] >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	return false;
}
bool BaseStepPathFinder::MapDataIsPass(BasePoint _mapGrid)
{
    int mapGridX = _mapGrid.x;
	int mapGridY = _mapGrid.y;
	
	if(mapGridX < 0 || mapGridX >= mapWidth)	return false;
	if(mapGridY < 0 || mapGridY >= mapHeight)	return false;
	
	if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		if(pMapData[mapGridY][mapGridX] >= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	return false;
}

void BaseStepPathFinder::MapDataClear()
{
    for(int i=0; i<ARRLEN(pMapData); i++)   // 行
	{
		for(int j=0; j<ARRLEN(pMapData[i]); j++) // 列
		{
			pMapData[i][j] = pMapDataInit[i][j];
		}
	}
}

int BaseStepPathFinder::FindAllPath(BasePoint _startMapGrid, int _maxMoveStep,  vector<BasePoint>* _outArr)
{
    if(NULL == _outArr) return 0;
	if(_maxMoveStep <= 0) return 0;
	
	// 是否越界
	if(_startMapGrid.x < 0 || _startMapGrid.x >= mapWidth)	return 0;
	if(_startMapGrid.y < 0 || _startMapGrid.y >= mapHeight)	return 0;
	
	_outArr->clear();// 先清除
	MapDataClear();
    
    MapDataSetPass(_startMapGrid, true);// 设置当前点可以通过
	SetAllPathToArray(_startMapGrid, _maxMoveStep, _outArr);
	
	return _outArr->size();
}
int BaseStepPathFinder::FindPath(BasePoint _startMapGrid ,BasePoint _targetMapGrid, int _maxMoveStep, vector<BasePoint>* _outArr)
{
    if(NULL == _outArr) return 0;
	if(_maxMoveStep <= 0) return 0;
	
	// 是否越界
	if(_startMapGrid.x < 0 || _startMapGrid.x >= mapWidth)	return 0;
	if(_startMapGrid.y < 0 || _startMapGrid.y >= mapHeight)	return 0;
	
	if(_targetMapGrid.x < 0 || _targetMapGrid.x >= mapWidth)	return 0;
	if(_targetMapGrid.y < 0 || _targetMapGrid.y >= mapHeight)	return 0;
	
    
    _outArr->clear();// 先清除
    MapDataClear();
    
    MapDataSetPass(_startMapGrid, true);    // 当前点可以过
    MapDataSetPass(_targetMapGrid, true);   // 目标点可以过
    
	
	//DebugPrintMapData();
    Find(_targetMapGrid, 0, _maxMoveStep);// 目标点高度从0 开始
	//DebugPrintMapData();
	
    
	// 成功找到目标点
	if( GetMapData(_startMapGrid) <  BaseStepPathFinder_MaxHeight)
	{
		currSearchHeight = GetMapData(_startMapGrid);
        SetPathToArray(_startMapGrid, _outArr);
	}
	
	return _outArr->size();
}
int BaseStepPathFinder::Find(BasePoint _targetMapGrid, int _currHeight, int _maxHeight)
{
    if(_currHeight > _maxHeight) return 0;
	
	// 是否越界
	if(_targetMapGrid.x < 0 || _targetMapGrid.x >= mapWidth)	return 0;
	if(_targetMapGrid.y < 0 || _targetMapGrid.y >= mapHeight)	return 0;
	
	// 当前网格点
	int mapGridX = _targetMapGrid.x;
	int mapGridY = _targetMapGrid.y;
	
	
	if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		// 不可以通过
		if(pMapData[mapGridY][mapGridX] < 0)
		{
			return 0;
		}
		// 不是最优路径
		if(_currHeight >= pMapData[mapGridY][mapGridX])
		{
			return 0;
		}
		
		pMapData[mapGridY][mapGridX] = _currHeight;
		
		// 递归调用(越往后，高度越大)
        Find(BasePoint(mapGridX-1, mapGridY), _currHeight+1, _maxHeight);  // 左边
        Find(BasePoint(mapGridX+1, mapGridY), _currHeight+1, _maxHeight);  // 右边
        Find(BasePoint(mapGridX, mapGridY+1), _currHeight+1, _maxHeight);  // 上边
        Find(BasePoint(mapGridX, mapGridY-1), _currHeight+1, _maxHeight);  // 下边
	}
	
	return 0;
}
int BaseStepPathFinder::SetPathToArray(BasePoint _startMapGrid,  vector<BasePoint>* _outArr)
{
    //if(NULL == _outArr) return 0;   // 在外面判断
	if(currSearchHeight < 0) return 0;
	
	// 是否越界
	if(_startMapGrid.x < 0 || _startMapGrid.x >= mapWidth)	return 0;
	if(_startMapGrid.y < 0 || _startMapGrid.y >= mapHeight)	return 0;
	
	// 当前网格点
	int mapGridX = _startMapGrid.x;
	int mapGridY = _startMapGrid.y;
	
	if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		// 不可以通过
		if(pMapData[mapGridY][mapGridX] < 0)
		{
			return 0;
		}
		
		// 非最优路径（不可以行走,只能往低处走）
		if(pMapData[mapGridY][mapGridX] > currSearchHeight)
		{
			return 0;
		}
		
		// 添加到行走数组中
        _outArr->push_back(BasePoint(mapGridX, mapGridY));
        
		currSearchHeight--;   // 地形高度
		
		// 递归调用 (不断向低处走)
        SetPathToArray(BasePoint(mapGridX-1, mapGridY), _outArr);     // 左边
        SetPathToArray(BasePoint(mapGridX+1, mapGridY), _outArr);     // 右边
        SetPathToArray(BasePoint(mapGridX, mapGridY+1), _outArr);     // 上边
        SetPathToArray(BasePoint(mapGridX, mapGridY-1), _outArr);     // 下边
	}
	
	return 0;
}
int BaseStepPathFinder::SetAllPathToArray(BasePoint _startMapGrid, int _maxMoveStep, vector<BasePoint>* _outArr)
{
    //if(NULL == _outArr) return 0; // 在外面判断
	if(_maxMoveStep < 0) return 0;
	
	// 是否越界
	if(_startMapGrid.x < 0 || _startMapGrid.x >= mapWidth)	return 0;
	if(_startMapGrid.y < 0 || _startMapGrid.y >= mapHeight)	return 0;
	
	// 当前网格点
	int mapGridX = _startMapGrid.x;
	int mapGridY = _startMapGrid.y;
    
    if(mapGridY < ARRLEN(pMapData) && mapGridX < ARRLEN(pMapData[mapGridY]) )  // 防止数组越界
	{
		// 不可以通过
		if(pMapData[mapGridY][mapGridX] < 0)
		{
			return 0;
		}
        
        // 当前网格已经被查找过
		if(pMapData[mapGridY][mapGridX] < BaseStepPathFinder_MaxHeight)
		{
            // 当前查找，是从远处返回来的 （注意，_maxMoveStep 越大，距离中心位置越近）
            if(_maxMoveStep <= pMapData[mapGridY][mapGridX])
            {
                return 0;
            }
		}
        
        // 第一次找到该点
        if(BaseStepPathFinder_MaxHeight == pMapData[mapGridY][mapGridX])
        {
            // 添加到行走数组中
            _outArr->push_back(BasePoint(mapGridX, mapGridY));
        }
        
        pMapData[mapGridY][mapGridX] = _maxMoveStep;  // 更新地势
        
		// 递归调用(越往后，高度越小)
        SetAllPathToArray(BasePoint(mapGridX-1, mapGridY), _maxMoveStep-1, _outArr);  // 左边
        SetAllPathToArray(BasePoint(mapGridX+1, mapGridY), _maxMoveStep-1, _outArr);  // 右边
        SetAllPathToArray(BasePoint(mapGridX, mapGridY+1), _maxMoveStep-1, _outArr);  // 上边
        SetAllPathToArray(BasePoint(mapGridX, mapGridY-1), _maxMoveStep-1, _outArr);  // 下边
	}
    
	return 0;
}
int BaseStepPathFinder::DebugPrintMapData()
{
#ifdef GAME_DEBUG_MODE
	
	printf("BaseStepPathFinder->DebugPrintMapData, Start \n");
	
	int mapGridY = 0;
	int mapGridX = 0;
	
	for (mapGridY=0; mapGridY< ARRLEN(pMapData) && mapGridY <1; mapGridY++)  // 打印 1行
	{
		printf("----------------------------------------  \n");
		for (mapGridX=0 ; mapGridX< ARRLEN(pMapData[mapGridY]); mapGridX++) // 列
		{
			printf("%02d", mapGridX);
		}
		printf("\n");
	}
    
    printf("pMapDataInit Table: \n");
	
	for (mapGridY=0; mapGridY< ARRLEN(pMapDataInit); mapGridY++)  // 行
	{
		printf("%02d ", mapGridY);
		
		for (mapGridX=0 ; mapGridX< ARRLEN(pMapDataInit[mapGridY]); mapGridX++) // 列
		{
			if(pMapDataInit[mapGridY][mapGridX] < 0)
			{
				printf("X ");  // 不可以通过
			}
			else if(pMapDataInit[mapGridY][mapGridX] < BaseStepPathFinder_MaxHeight)
			{
				printf("* "); // 已经查找过
			}
			else
			{
				printf("@ "); // 可以通过
			}
			
		}
		
		printf("\n");
	}
	
	
	printf("pMapData Table: value \n");
	for (mapGridY=0; mapGridY< ARRLEN(pMapData); mapGridY++)  // 行
	{
		printf("%02d ", mapGridY);
		
		for (mapGridX=0 ; mapGridX< ARRLEN(pMapData[mapGridY]); mapGridX++) // 列
		{
			if(pMapData[mapGridY][mapGridX] < 0)
			{
				printf("X ");  // 不可以通过
			}
			else if(pMapData[mapGridY][mapGridX] < BaseStepPathFinder_MaxHeight)
			{
				printf("%02d", pMapData[mapGridY][mapGridX]); // 已经查找过
			}
			else
			{
				printf("@ "); // 可以通过
			}
		}
		
		printf("\n");
	}
	
	
	printf("pMapData Table: (通行能力, O 表示可以行走点) \n");
	for (mapGridY=0; mapGridY< ARRLEN(pMapData); mapGridY++)  // 行
	{
		printf("%02d ", mapGridY);
		
		for (mapGridX=0 ; mapGridX< ARRLEN(pMapData[mapGridY]); mapGridX++) // 列
		{
			if(pMapData[mapGridY][mapGridX] < 0)
			{
				printf("X ");  // 不可以通过
			}
			else if(pMapData[mapGridY][mapGridX] < BaseStepPathFinder_MaxHeight)
			{
				printf("* "); // 已经查找过
			}
			else
			{
				printf("@ "); // 可以通过
			}
            
		}
		
		printf("\n");
	}
	
	printf("BaseStepPathFinder->DebugPrintMapData, End \n\n\n");
	
#endif
	
	return 0;
}