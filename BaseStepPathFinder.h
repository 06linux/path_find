
/*
 *  BaseStepPathFinder.h
 *  CCGame
 *
 *  Created by linux_wuliqiang@163.com on 2015-03-30.
 *  Copyright 2015 Beijing. All rights reserved.
 *
 *	最优寻路算法，可以设置最多行走步数限制
 *
 *	思路说明：http://blog.chinaunix.net/space.php?uid=20622737&do=blog&id=1912766
 *
 *	备注说明：寻路前，必须先 MapDataClear 重置地图数据，确保一个干净的地图寻路数据
 *
 *	使用举例：




 任务： 从 A 点移动到，B点，画 X 的表示不能通过，计算最优路径。入下图所示：

        如上图所示，最优的路径为红线表示路径。计算方法如下：
        1>     地图数据可以看做一个个的正方形格子。
        2>     将所有地图格子的数值都初始化为 1000 （假设最多移动1000步）
        3>     将画 X 的格子，设置值为 -1，表示此格子不能通过
        4>     将 B 点出格子的数值设置为 0，然后向四周发散，如果某个的格子的数值比 B 点的大，则将此处的格子值设置为 “前一个格子的值 + 1”，然后以次格子为中心，以此向外发散。
        5>     最后，找到 A 点时，A 点处的格子的值为 14，此时，从A到 B的最优路径为：14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        6>     说明，此寻路算法为比较简单的寻路算法，比较适合处理速度有限的嵌入式开发平台。
 *
 *
 */
#import "Base.h"
#ifndef __BaseStepPathFinder_H__
#define __BaseStepPathFinder_H__
#define  BaseStepPathFinder_MaxHeight	1000		// 寻路最大高度
class BaseStepPathFinder
{
    
public:     // 构造析构函数
    BaseStepPathFinder();      
    ~BaseStepPathFinder();
    
    static BaseStepPathFinder* Instance();
    
public:     
    
    /*
     *  寻路前，必须调用 InitMapData 先进行初始化，然后才可以寻路
     */
    void        InitMapData(int _mapWidth, int _mapHeight);
    void        FreeMapData();
    
    /*
     *  设置地图数据中的某一个点，是否可以通过
     */
    void        SetPass(BasePoint _mapGrid, bool _isCanPass);
    
    /*
     *  网格点是否可以通过
     */
    bool        IsPass(BasePoint _mapGrid);
    
    /*
     *	函数说明：	寻路查找，输出最优路径到数组中
     *	参数说明：	_startMapGrid, 起始网格点
     *				_targetMapGrid, 目标网格点
     *				_maxMoveStep, 限制最大的移动步数
     *				_outArray, （输出参数）寻路查找的结果, 存储 BasePointInt 对象
     *	函数返回：	返回 _outArray 数组中 元素的个数
     *
     *	备注说明：	地势，从低到高查找
     *
     *
     */
    int         FindPath(BasePoint _startMapGrid ,BasePoint _targetMapGrid, int _maxMoveStep, vector<BasePoint>* _outArr);
    
    
    
    /*
     *	函数说明：	输出所有的可以行走网格点到数组中, 以 _startMapGrid 点为中心点， _maxMoveStep 为移动步数，输出所有的可以移动到的网格点
     *	参数说明：	_startMapGrid, 起始网格点
     *				_maxMoveStep, 限制最大的移动步数
     *				_outArray, （输出参数）寻路查找的结果, 存储 BasePointInt 对象
     *	函数返回：	返回 _outArray 数组中 元素的个数
     *
     *	备注说明：	地势从高向低查找
     *
     */
    int         FindAllPath(BasePoint _startMapGrid, int _maxMoveStep,  vector<BasePoint>* _outArr);
    
    
  
    
    /*
     *  调试输出
     */
    int         DebugPrintMapData();
    
private:
    
    void        SetMapData(BasePoint _mapGrid, int _value);
    int         GetMapData(BasePoint _mapGrid);
    
    
    /*
     *	函数说明：	寻路查找，在查找过程中，不断设置地形的高度
     *	参数说明：	_targetMapGrid, 目标网格点，在开始的时候，设置地形高度为 0
     *				_maxHeight, 限制最大的移动步数
     *	函数返回：	0
     *
     *	备注说明：	每增加一步，高度 +1
     *
     */
    int         Find(BasePoint _targetMapGrid, int _currHeight, int _maxHeight);
    
    /*
     *  类内部调用, 从高地形，向低地形查找
     */
    int         SetPathToArray(BasePoint _startMapGrid,  vector<BasePoint>* _outArr);
    
    /*
     *  查找并设置所有的可以行走的点 (地势从高向低进行查找)
     *  注意，_maxMoveStep 越大，距离中心位置越近
     *  _maxMoveStep 越小，距离中心位置越远
     */
    int         SetAllPathToArray(BasePoint _startMapGrid, int _maxMoveStep, vector<BasePoint>* _outArr);
    
    /*
     *  清除寻路数据为初始化状态
     */
    void        MapDataClear();
    
    /**
     *  临时寻路数据设置
     */
    void        MapDataSetPass(BasePoint _mapGrid, bool _isCanPass);
    bool        MapDataIsPass(BasePoint _mapGrid);
    
private:
    
    int			mapWidth;		// 地图的宽度
	int			mapHeight;		// 地图的高度
	
	int**		pMapData;		// 地图数据（－1，表示不能通过 >=0, 表示可以通过）--- 当前使用的寻路地图 (临时寻路数据)
	int**		pMapDataInit;	// 地图数据 --- 初始的地图通行状态
	
	int			currSearchHeight;	// 当前搜索的地形的高度
    
};
#endif // __BaseStepPathFinder_H__
